// Target module

use crate::dependency::Dependency;

pub const TARGET_DEFAULT: &str = "default";

pub struct Target {
    pub name: String,
    pub deps: Vec<Dependency>,
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
