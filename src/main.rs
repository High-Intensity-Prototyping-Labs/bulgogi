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
                    let module = sub.get_one::<String>("MODULE").unwrap_or(&String::from(MODULE_DEFAULT));
                    let target = sub.get_one::<String>("TARGET").unwrap_or(&String::from(TARGET_DEFAULT));
                }
                _ => unreachable!(),
            }
        }
        _ => unreachable!(),
    }
}
