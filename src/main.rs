use std::ffi::OsString;
use clap::{arg, Command};
use serde::{Serialize, Deserialize};

struct Project {
    targets: Vec<Target>,
}

struct Target {
    name: String,
    deps: Vec<Dependency>
}

enum Module {
    Name(String),
}

enum Dependency {
    Module(Module),
    Target(Target),
}
use serde_yaml::{Mapping};

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
            let project = Project {
                targets: vec![
                    Target { 
                        name: String::from("default"), 
                        deps: vec![
                            Dependency::Module(Module::Name (
                               String::from("module1"),
                            )),
                            Dependency::Module(Module::Name (
                                String::from("module2"),
                            )),
                        ]
                    },
                    Target { 
                        name: String::from("target1"), 
                        deps: vec![
                            Dependency::Module(Module::Name (
                               String::from("module1"),
                            )),
                            Dependency::Module(Module::Name (
                                String::from("module2"),
                            )),
                        ]
                    },
                ],
            };

            let mut m = Mapping::new();
            m.insert("default".into(), vec!["module1", "module2"].into());
            m.insert("target1".into(), vec!["module3", "module4"].into());
            m.insert("target2".into(), vec!["module1", "target1"].into());
            let yaml = serde_yaml::to_string(&m);

            println!(
                "Initialized bulgogi project {}",
                sub_matches.get_one::<String>("PATH").expect("required")
            );

            println!("Serialized yaml string:\n{}", yaml.expect("error"));
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
