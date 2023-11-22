// Root module
mod project;
mod target;
mod dependency;
mod client;

use crate::project::Project;
use crate::target::TARGET_DEFAULT;
use crate::dependency::MODULE_DEFAULT;

fn main() {
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
                    let module = sub.get_one::<String>("MODULE").unwrap_or(&String::from(MODULE_DEFAULT));
                    let target = sub.get_one::<String>("TARGET").unwrap_or(&String::from(TARGET_DEFAULT));

                    // Load project
                    let mut project = Project::load().unwrap_or(Project::new());

                    // Add module
                    project.add_target(target);

                    // TODO:
                    // Come to terms with how the high-level interface is fighting with itself;
                    // The project methods are a little too independent because a lot of the
                    // default behaviour can be handled up here.
                    //
                    // Decide to either:
                    //  1. Handle less defaults and assumptions in the Project interface 
                    //  2. Do much less up here
                }
                _ => unreachable!(),
            }
        }
        _ => unreachable!(),
    }
}
