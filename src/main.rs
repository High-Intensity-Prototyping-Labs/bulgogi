use std::ffi::OsString;
use std::io::prelude::*;
use std::fs::File;
use clap::{arg, Command};
use serde::{Serialize, Deserialize};
use serde_yaml::{Mapping, Error, Value, Sequence};

struct Project {
    targets: Vec<Target>,
}

#[derive(Serialize, Deserialize, Clone)]
struct Target {
    name: String,
    deps: Vec<Dependency>
}

#[derive(Serialize, Deserialize, Clone)]
struct Module {
    name: String,
}

#[derive(Serialize, Deserialize, Clone)]
#[serde(untagged)]
enum Dependency {
    Module(Module),
    Target(Target),
}

impl Project {
    fn new() -> Project {
        Project {
            targets: vec![],
        }
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

        // 1. First pass -- assume all deps to be modules
        for (key, value) in mapping.iter() {
            if let (Value::String(target), Value::Sequence(deps)) = (key, value) {
                project.targets.push(
                    Target { 
                        name: target.clone(), 
                        deps: deps.iter().map(|d| Dependency::from(d)).collect(),
                    }
                );
            }
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

fn main() {
    let matches = cli().get_matches();

    match matches.subcommand() {
        Some(("init", sub_matches)) => {
            // Test project structure
            let project = Project {
                targets: vec![
                    Target { 
                        name: String::from("default"), 
                        deps: vec![
                            Dependency::Module(Module { name: String::from("module1") }),
                            Dependency::Module(Module { name: String::from("module2") }),
                        ]
                    },
                    Target { 
                        name: String::from("target1"), 
                        deps: vec![
                            Dependency::Module(Module { name: String::from("module1") }),
                            Dependency::Module(Module { name: String::from("module2") }),
                        ]
                    },
                ],
            };

            // Now using the fancy rust types
            let fancy_map = Mapping::from(project);
            let fancy_yaml = serde_yaml::to_string(&fancy_map);
            println!("Fancy yaml project:\n{}", fancy_yaml.expect("error"));

            // Load project.yaml file 
            let mut f = File::open("project.yaml").expect("failed to open file");
            let map_f: Mapping = serde_yaml::from_reader(f).expect("error");
            let proj = Project::from(map_f);
        }
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
