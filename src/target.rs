// Target module

use crate::dependency::Dependency;

pub struct Target {
    pub name: String,
    pub deps: Vec<Dependency>,
}
