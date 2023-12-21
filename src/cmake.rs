// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module, ModuleID, ProjectChild};

use std::collections::HashMap;

pub type Submodule = String;

pub struct CMakeProject {
    pub submodules: Vec<Submodule>,
    pub lists: HashMap<Submodule, CMakeList>,
}

pub struct CMakeList {
    pub target: CMakeTarget,
    pub links: Option<>,
}

pub enum CMakeTarget {
    Library(String),
    Executable(String),
}
