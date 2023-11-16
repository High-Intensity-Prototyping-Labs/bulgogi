use std::ffi::OsString;
use std::fs::File;
use std::io::ErrorKind;
use std::io;
use std::path::Path;
use clap::{arg, Command};
use serde::{Serialize, Deserialize};
use serde_yaml::{Mapping, Value};

#[derive(Debug, Clone)]
struct Project {
    targets: Vec<Target>,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
struct Target {
    name: String,
    deps: Vec<Dependency>,
    kind: TargetKind,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
enum TargetKind {
    Binary,
    Library,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
enum Dependency {
    Module(String),
    Target(String),
}

impl Project {
    fn new() -> Project {
        Project {
            targets: vec![],
        }
    }

    fn check_cylic(&self) -> bool {
        // Start with the first target. For each dependency of type target,
        // follow its dependencies. For each of those dependencies of type target,
        // repeat and continue the sequence. At every step, deliberately test the
        // name of the target against the original target that is being checked.
        // **IF EVER** there is a match down the road, this necessarily means that directly
        // or indirectly, one of the OG target's dependencies include IT, thus circular.

        for target in &self.targets {
            if target.check_deps(self, None) {
                return true;
            }
        }
        false
    }

    fn find(&self, name: String) -> Option<&Target> {
        for target in &self.targets {
            if target.name == name {
                return Some(&target);
            }
        }
        None
    }

    /// Initializes the module directories in the FS.
    /// Ignores any missing entries in project.yaml (true negatives).
    ///
    /// # Arguments
    ///
    /// * `root` - Root directory of the project to spawn.
    fn spawn(&self, root: &Path) {
        for target in &self.targets {
            for dep in &target.deps {
                // Match against module-type dependencies
                if let Dependency::Module(m) = dep {
                    // Assume project.yaml path = PWD
                    let path = Path::new(root).join(m);

                    // Make sure module path doesn't exist
                    if !path.exists() {
                        // Create source and include subdirectories
                        let src_path = path.join("src");
                        let inc_path = path.join("inc");
                        let private_inc_path = src_path.join("inc");

                        std::fs::create_dir_all(src_path).expect("required");
                        std::fs::create_dir_all(inc_path).expect("required");
                        std::fs::create_dir_all(private_inc_path).expect("required");
                    }
                }
            }
        }
    }
}

impl Target {
    fn check_deps(&self, project: &Project, parent: Option<&Target>) -> bool {
        // Checks through its own dependencies.
        // First by testing one level down into its target-type dependencies,
        // then if any target-type dependencies are found at that level, recursively
        // call the check_deps function.
        // First (top-level) caller should pass None as parent Option, subsequent recursive calls
        // must pass the parent down as this is the determining criterion for the check.
        //
        // Returns true if a match for the parent is found
    
        // For each dependency in self target
        for dep in &self.deps {
            // Match the depdencies of the target kind
            if let Dependency::Target(target_dep) = dep {
                // Find the actual target struct from the project
                if let Some(t) = project.find(target_dep.into()) {
                    // Recurse into dependencies of found target-type dependency
                    for recursed_dep in &t.deps {
                        // Match the target-type dependencies in the recursed target
                        if let Dependency::Target(recursed_target_dep) = recursed_dep {
                            // Retrieve the actual target struct of the matched target-type
                            // dependencies
                            if let Some(recursed_t) = project.find(recursed_target_dep.into()) {
                                // Check if current target dependency's target-dependency matches
                                // parent name 
                                if recursed_t.name == parent.unwrap_or(self).name {
                                    return true;
                                }

                                // Continue down the road for the target's target dependency
                                else if recursed_t.check_deps(project, parent.or(Some(self))) {
                                    return true;
                                }

                                // Otherwise move on to the next target-dependency's
                                // target-dependency 
                                else { continue };
                            }
                        }
                    }
                }
            }
        }
        false
    }

    fn find(&self, name: &String) -> Option<&Dependency> {
        for dep in &self.deps {
            if &String::from(dep.clone()) == name {
                return Some(&dep);
            }
        }
        None
    }
}

impl Default for Project {
    fn default() -> Self {
        Project {
            targets: vec![Target::default()],
        }
    }
}

impl Default for Target {
    fn default() -> Self {
        Target {
            name: String::from("default"),
            deps: vec![Dependency::default()],
            kind: TargetKind::Binary,
        }
    }
}

impl Default for Dependency {
    fn default() -> Self {
        Dependency::Module(String::from("module1"))
    }
}

impl From<Vec<Target>> for Project {
    fn from(targets: Vec<Target>) -> Self {
        Project {
            targets,
        }
    }
}

impl From<Mapping> for Project {
    fn from(mapping: Mapping) -> Self {
        let mut project = Project::new();

        for (key, value) in mapping {
            if let (Value::String(key_string), Value::Sequence(value_seq)) = (key, value) {
                project.targets.push(
                    Target {
                        name: key_string,
                        kind: TargetKind::Binary,
                        deps: value_seq.into_iter().map(|v| Dependency::from(v)).collect(),
                    }
                );
            }
        }

        // Re-map dependencies into their corresponding Module or Target variants 
        let ref_project = project.clone();
        for target in &mut project.targets {
            // Convert dependencies to ::Target type if a target is found with the 
            // same name.
            for dep in &mut target.deps {
                /* dep can be Dependency::Module or Dependency::Target type */
                if let Dependency::Module(m) = dep {
                    if let Some(_) = ref_project.find(m.clone()) {
                        /* we've found a match for the dependency in the project's targets */
                        // Update the current dep's type to Dep::Target
                        *dep = Dependency::Target(m.clone());
                    }
                }
            }

            // Then convert target kinds to library when target dependencies 
            // adjacent to it contain itself as a dependency (inferring library vs binary).
            for search_target in &ref_project.targets {
                if let Some(_) = search_target.find(&target.name) {
                    target.kind = TargetKind::Library;
                }
            }
        }

        // Check for cyclic dependencies 
        if project.check_cylic() {
            help(HelpKind::CyclicDependency);
            panic!("cyclic dependency");
        }

        project
    }
}

impl From<Value> for Dependency {
    fn from(value: Value) -> Self {
        if let Value::String(val_string) = value {
            Dependency::Module(val_string)
        } else {
            Default::default()
        }
    }
}

impl From<Dependency> for String {
    fn from(dep: Dependency) -> Self {
        match dep {
            Dependency::Module(m) => m,
            Dependency::Target(t) => t,
        }
    }
}

// Add support for implicit conversion from Dependency to YAML Value
impl From<Dependency> for Value {
    fn from(dep: Dependency) -> Self {
        Value::String(dep.into())
    }
}

// Add support for implicit conversion from Project to YAML Mapping
impl From<Project> for Mapping {
    fn from(value: Project) -> Self {
        let mut m = Mapping::new();
        for target in &value.targets {
            m.insert(target.name.clone().into(), target.deps.clone().into());
        }
        m
    }
}

fn cli() -> Command {
    Command::new("bulgogi")
        .about("A build system orchestrator for CubeSat projects")
        .subcommand_required(true)
        .arg_required_else_help(true)
        .allow_external_subcommands(true)
        .subcommand(
            Command::new("init")
                .about("Initializes bulgogi project directory")
                .arg(arg!(<PATH> "The directory to initialize"))
                .arg_required_else_help(true),
        )
        .subcommand(
            Command::new("spawn")
                .about("Spawns project directory structure and ignores missing entries")
        )
        .subcommand(
            Command::new("module")
            .about("Manage project modules")
            .arg_required_else_help(true)
            .subcommand(
                Command::new("add")
                .about("Adds a new module to the bulgogi project")
                .arg(arg!(<PATH> "Path to the module in question"))
                .arg(arg!(--create))
            )
            .subcommand(
                Command::new("rm")
                .about("Removed a module from the bulgogi project")
                .alias("remove")
                .arg(arg!(<PATH> "Path to the module in question"))
                .arg(arg!(--cached))
            )
        )
        .subcommand(
            Command::new("build")
            .about("Builds the bulgogi project")
        )
        .subcommand(
            Command::new("clean")
            .about("Cleans the local project build files and cache")
        )
        .subcommand(
            Command::new("test")
            .about("Performs test functions -- used for development.")
        )
}

enum InfoKind {
    InitSuccess,
    SpawnSuccess,
}

fn info(msg: InfoKind) {
    match msg {
        InfoKind::InitSuccess => println!("Successfully initialized project."),
        InfoKind::SpawnSuccess => println!("Successfully spawned project directories."),
    }
}

enum HelpKind {
    CyclicDependency,
    ProjectFound,
    NotInitialized,
    InitRequired,
}

fn help(msg: HelpKind) {
    match msg {
        HelpKind::CyclicDependency => println!("A cyclic dependency in your project.yaml file was detected. Consider fixing this and trying again."),
        HelpKind::ProjectFound => println!("Found project.yaml -- no need to initialize."),
        HelpKind::NotInitialized => println!("Cannot find a project.yaml in the current directory."),
        HelpKind::InitRequired => println!("An initialized project is required to continue."),
    }
}

fn load(root: &String) -> Option<Project> {
    let dir = Path::new(root).join("project.yaml");
    let file = File::open(dir);

    match file {
        Ok(f) => {
            let map: Mapping = serde_yaml::from_reader(f).expect("required");
            let project = Project::from(map);
            Some(project)
        }
        Err(_) => None,
    }
}

fn init(dir: &String) {
    // Load project.yaml file 
    let path = Path::new(dir).join("project.yaml");
    let file = File::open(&path);

    match file {
        Ok(_) => {
            help(HelpKind::ProjectFound);
        }
        Err(e) => {
            match e.kind() {
                ErrorKind::NotFound => {
                    // Create project directory
                    std::fs::create_dir_all(dir).expect("Could not create dir");

                    // Create default project
                    let project = Project::default();
                    let new_file = File::create(path).expect("Could not create new file");
                    let new_map = Mapping::from(project.clone());
                    serde_yaml::to_writer(new_file, &new_map).expect("Failed to write default project to file");

                    // Spawn project directory structure 
                    project.spawn(&Path::new(dir));

                    info(InfoKind::InitSuccess);
                }
                _ => panic!("Unable to open project.yaml file"),
            }
        }
    }

}

fn spawn() {
    // Load project
    if let Some(project) = load(&String::from(".")) {
        project.spawn(Path::new("."));
        info(InfoKind::SpawnSuccess);
    } else {
        // Prompt user to init project 
        help(HelpKind::NotInitialized);
        print!("Automatically initialize project in the current directory? (Y/n): ");

        let mut answer = String::new();
        io::stdin().read_line(&mut answer).expect("stdio");
        if answer.trim() == "Y" || answer.trim() == "y" {
            // Try the whole thing again after init if user agrees
            init(&String::from("."));
            spawn();
        } else {
            help(HelpKind::InitRequired);
        }
    }

}

fn test() {
}

fn main() {
    let matches = cli().get_matches();

    match matches.subcommand() {
        Some(("init", sub_matches)) => init(&sub_matches.get_one::<String>("PATH").expect("required")),
        Some(("spawn", _)) => spawn(),
        Some(("module", sub_matches)) => {
            match sub_matches.subcommand() {
                Some(("add", sub2_matches)) => {
                    let create = sub2_matches.get_flag("create");

                    println!(
                        "Adding module {} (--create = {})",
                        sub2_matches.get_one::<String>("PATH").expect("required"),
                        create,
                    );
                }
                Some(("rm", sub2_matches)) => {
                    let cached = sub2_matches.get_flag("cached");

                    println!(
                        "Removing project module {} (--cached = {})",
                        sub2_matches.get_one::<String>("PATH").expect("required"),
                        cached,
                    );
                }
                _ => unreachable!(),
            }
        }
        Some(("build", _)) => {
            println!(
                "Building bulgogi project..."
            );
        }
        Some(("clean", _)) => {
            println!(
                "Cleaning bulgogi project..."
            );
        }
        Some(("test", _)) => test(),
        Some((ext, sub_matches)) => {
            let args = sub_matches
                .get_many::<OsString>("")
                .into_iter()
                .flatten()
                .collect::<Vec<_>>();
            println!("Calling out to {ext:?} with {args:?}");
        }
        _ => unreachable!(),
    }

    println!("Hello, world!");
}
