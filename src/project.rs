// Project module
use crate::target::Target;
use crate::client;
use crate::client::{InfoKind, HelpKind};

use std::fs::File;
use std::path::Path;

pub struct Project {
    targets: Vec<Target>,
}

impl Project {
    pub fn new() -> Self {
        Project {
            targets: vec![],
        }
    }

    /// Initializes a bulgogi project in the PWD of the shell.
    pub fn init() {
        // Check if project.yaml exists 
        if Path::new("project.yaml").exists() {
            // Advise that project exists
            client::help(client::HelpKind::ProjectFound);
        } else {
            // Create project file with nothing in it
            if let Ok(_) = File::create("project.yaml") {
                // Project initialized
                client::info(InfoKind::InitSuccess);
            } else {
                // Could not create project.yaml 
                client::help(HelpKind::ProjectInitFailed);
            }
        }
    }
}
