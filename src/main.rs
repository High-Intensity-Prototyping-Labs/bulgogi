// Root module
mod project;
mod target;
mod dependency;
mod client;

use crate::client::{HelpKind, InfoKind};
use crate::project::Project;
use crate::target::TARGET_DEFAULT;
use crate::dependency::MODULE_DEFAULT;

use std::io;

fn main() -> Result<(), io::Error> {
    let matches = client::cli().get_matches();

    match matches.subcommand() {
        Some(("init", _)) => {
            // Initialize project in the PWD
            Project::init();
        }
        Some(("module", cmd)) => {
            match cmd.subcommand() {
                Some(("add", sub)) => {
                    // Placehold add command arguments
                    let module = sub.get_one::<String>("MODULE").expect("required").to_owned();
                    let target = sub.get_one::<String>("TARGET").expect("required").to_owned();

                    // Load project
                    match Project::load() {
                        Ok(mut project) => {
                            if project.has_module(&module) {
                            // Duplicate found, notify
                                client::info(InfoKind::DuplicateModule);
                            } else {
                            // No duplicates found, continue
                                // Add module to project
                                project.add_module(target, module.clone());

                                // Spawn module directory 
                                Project::spawn_module(module)?;

                                // Save project 
                                project.save().expect("yaml");

                                // Notify success 
                                client::info(InfoKind::AddModuleSuccess);
                            }
                        }
                        Err(e) => {
                            if e.kind() == io::ErrorKind::NotFound {
                                client::help(HelpKind::InitRequired);
                            } else {
                                println!("{}", e);
                            }
                        }
                    }
                }
                _ => unreachable!(),
            }
        }
        Some(("tree", _)) => {
            // Load project 
            match Project::load() {
                Ok(project) => {
                    project.tree();
                }
                Err(e) => {
                    if e.kind() == io::ErrorKind::NotFound {
                        client::help(HelpKind::InitRequired);
                    } else {
                        println!("{}", e);
                    }
                }
            }
        }
        _ => unreachable!(),
    }
    Ok(())
}
