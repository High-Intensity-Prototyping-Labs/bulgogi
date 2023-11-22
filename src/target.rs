// Target module

use crate::dependency::Dependency;

pub struct Target {
    name: String,
    deps: Vec<Dependency>,
}
