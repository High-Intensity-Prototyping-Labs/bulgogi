use std::ffi::OsString;
use std::fs::File;
use std::io::ErrorKind;
use std::path::Path;
use clap::{arg, Command, ArgMatches};
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
}

impl Target {
    fn check_deps(&self, project: &Project, mut parent: Option<&Target>) -> bool {
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
        }
        
        // Check for cyclic dependencies 
        if project.check_cylic() {
            panic!("Cyclic dependencies detected -- cannot proceed");
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
}

fn init(matches: &ArgMatches) {
    // Load project.yaml file 
    let dir = matches.get_one::<String>("PATH").expect("error");
    let path = Path::new(dir).join("project.yaml");
    let file = File::open(&path);

    match file {
        Ok(_) => {
            // Project exists, notify user that init was useless.
            println!("Found project.yaml -- no need to initialize ({:?}).", path);
        }
        Err(e) => {
            match e.kind() {
                ErrorKind::NotFound => {
                    // Create directory structure

                    // Create default project
                    let new_file = File::create("project.yaml").expect("Could not create new file");
                    let new_map = Mapping::from(Project::default());
                    serde_yaml::to_writer(new_file, &new_map).expect("Failed to write default project to file");

                    // Test created file
                    println!("Could not find project.yaml -- default project loaded and saved:\n{:#?}", Project::default());
                }
                _ => panic!("Unable to open project.yaml file"),
            }
        }
    }

}

fn main() {
    let matches = cli().get_matches();

    match matches.subcommand() {
        Some(("init", sub_matches)) => init(&sub_matches),
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
