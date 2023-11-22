// Target module

use crate::dependency::{Dependency, DepKind};

pub const TARGET_DEFAULT: &str = "default";

#[derive(Debug, Clone)]
pub struct Target {
    pub name: String,
    pub deps: Vec<Dependency>,
}

impl Target {
    /// Adds a module to this target 
    pub fn add(&mut self, module: String) {
        self.deps.push((module, DepKind::Module));
    }
}

impl Default for Target {
    fn default() -> Self {
        Target {
            name: String::from(TARGET_DEFAULT),
            deps: vec![],
        }
    }
}

impl From<String> for Target {
    fn from(value: String) -> Self {
        Target {
            name: value,
            ..Default::default()
        }
    }
}
