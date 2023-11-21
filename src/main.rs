// Root module
mod project;
mod target;
mod client;

use crate::project::Project;

fn main() {
    let matches = client::cli().get_matches();

    match matches.subcommand() {
        Some(("init", _)) => {
            // Initialize project in the PWD
            Project::init();
        }
        _ => unreachable!(),
    }
}
