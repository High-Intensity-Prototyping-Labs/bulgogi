// Root module
mod project;
mod target;
mod dependency;
mod client;
mod template;
mod cmake;

use crate::client::{HelpKind, InfoKind};
use crate::project::Project;

use std::io;

/// Shorthand to get an argument from a cli match
macro_rules! get_one {
    ($x:expr, $y:ty, $z:expr) => {
        $x.get_one::<$y>($z).expect("required").to_owned()
    };
}

/// Shorthand to get true/false state of flag from cli match
macro_rules! get_flag {
    ($x:expr, $y:expr) => {
        $x.get_flag($y)
    }
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

/// Displays a tree diagram of the project modules 
fn cli_tree() {
    // Load project 
    match Project::load() {
        Ok(project) => {
            // Make sure project has at least 1 target
            if !project.targets.is_empty() {
                project.tree();
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

/// Removes a module from the project 
fn cli_rm_module(target: String, module: String, cached: bool) {
    // Load project
    if let Ok(mut project) = Project::load() {
        project.rm_module(target, module, cached);
        project.save().expect("yaml");
    } else {
        client::help(HelpKind::ProjectLoadFailed);
    }
}

/// Loads a template from the templates dir 
fn cli_template() {
    template::load().expect("error");
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
                Some(("rm", sub)) => {
                    // Placehold add command arguments
                    let module = get_one!(sub, String, "MODULE");
                    let target = get_one!(sub, String, "TARGET");
                    let cached = get_flag!(sub, "cached");
                    cli_rm_module(target, module, cached);
                }
                _ => unreachable!(),
            }
        }
        Some(("tree", _)) => {
            cli_tree();
        }
        Some(("template", _)) => {
            cli_template();
        }
        _ => unreachable!(),
    }
    Ok(())
}
