// Root module
mod project;
mod target;
mod dependency;
mod client;

use crate::client::HelpKind;
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
                            // Add module to project
                            project.add_module(target, module.clone());

                            // Spawn module directory 
                            Project::spawn_module(module)?;

                            // Save project 
                            project.save().expect("yaml");

                        }
                        Err(e) => {
                            client::help(HelpKind::InitRequired);
                            println!("{}", e);
                        }
                    }
                }
                _ => unreachable!(),
            }
        }
        _ => unreachable!(),
    }
    Ok(())
}
