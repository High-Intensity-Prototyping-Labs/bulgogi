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
    deps: Vec<Dependency>
}

#[derive(Serialize, Deserialize, Clone, Debug)]
struct Module {
    name: String,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
#[serde(untagged)]
enum Dependency {
    Module(Module),
    Target(Target),
    Name(String),
}

impl Project {
    fn new() -> Project {
        Project {
            targets: vec![],
        }
    }

    fn check_circ_dep(&self) -> bool {
        for target in &self.targets {
            for dep in &target.deps {
                if let Dependency::Target(target_dep) = dep {
                    for dep_dep in &target_dep.deps {
                        if let Dependency::Name(dep_dep_name) = dep_dep {
                            if &target.name == dep_dep_name {
                                return true;
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
        }
    }
}

impl Default for Dependency {
    fn default() -> Self {
        Dependency::Module(
            Module {
                name: String::from("module1"),
            }
        )
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

        // First pass -- does not infer targets against dependencies
        for (key, value) in mapping.iter() {
            if let (Value::String(target), Value::Sequence(deps)) = (key, value) {
                // Create empty vector of dependency names
                let mut deps_str: Vec<String> = Vec::new();

                // Convert the entries in the YAML Value Sequence into strings for the 
                // vector of dependency names
                for dep in deps {
                    match dep {
                        Value::String(s) => {
                            deps_str.push(String::from(s));
                        }
                        _ => unreachable!(),
                    }
                }

                // Create the target to append to the project using the vector of dependency 
                // names.
                let target = Target {
                    name: String::from(target),
                    deps: deps_str.into_iter().map(|ds| Dependency::Name(ds)).collect(),
                };

                project.targets.push(target);
            }
        }

        // Second pass -- infer which dependencies are targets and which are modules
        let ref_project = project.clone();
        for target in &mut project.targets {
            for dep in &mut target.deps {
                let mut found = false;
                if let Dependency::Name(dep_name) = dep.clone() {
                    for search_target in ref_project.clone().targets {
                        if dep_name.to_owned() == search_target.name {
                            *dep = Dependency::Target(search_target);
                            found = true;
                        }
                    }
                    if !found {
                        *dep = Dependency::Module(Module{ name: dep_name });
                    }
                }
            }
        }

        // Check for circular dependencies
        if project.check_circ_dep() {
            panic!("Circular dependencies detected, cannot proceed");
        }

        project
    }
}

impl From<Value> for Dependency {
    fn from(value: Value) -> Self {
        match value {
            Value::String(str) => {
                Dependency::Module(
                    Module {
                        name: str,
                    }
                )
            }
            _ => unreachable!(),
        }
    }
}

// Add support for implicit conversion from Dependency to YAML Value
impl From<Dependency> for Value {
    fn from(value: Dependency) -> Self {
        Value::String(
            match value {
                Dependency::Module(module) => {
                    module.name
                }
                Dependency::Target(target) => {
                    target.name
                }
                Dependency::Name(name) => {
                    name
                }
            }
        )
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
