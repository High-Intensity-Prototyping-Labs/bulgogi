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

/// Shorthand to get an argument from a cli match
macro_rules! get_one {
    ($x:expr, $y:ty, $z:expr) => {
        $x.get_one::<$y>($z).expect("required").to_owned()
    };
}

/// High-level cli func to add a module to the project in the PWD.
fn cli_add_module(target: String, module: String) -> Result<(), io::Error> {
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
    Ok(())
}

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
                    let module = get_one!(sub, String, "MODULE");
                    let target = get_one!(sub, String, "TARGET");
                    cli_add_module(target, module)?;
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
