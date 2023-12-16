// CMake Module

use crate::target::Target;
use crate::project::Project;
use crate::dependency::{Dependency, DepFlag, DepKind};

use std::collections::HashMap;

pub struct CMakeProject {
    pub targets: Vec<CMakeList>,
}

pub type CMakeList = HashMap<String, CMakeTarget>;

pub enum CMakeTarget {
    LibModule(String),
    LibTarget(String, String),
    ExeTarget(String, String),
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            targets: Vec::new(),
        }
    }
}

/// Bulgogi projects and CMakeProjects are considered interchangeable.
impl From<Project> for CMakeProject {
    fn from(project: Project) -> Self {
        for target in project.targets() {
            
        }
        CMakeProject::new()
    }
}

impl From<(Target, Dependency)> for CMakeTarget {
    fn from(value: (Target, Dependency)) -> Self {
        CMakeTarget::LibModule(String::from("wow"))
    }
}

/// Each abstract bulgogi target yields a CMakeList
impl From<Target> for CMakeList {
    fn from(target: Target) -> Self {
        let mut list = CMakeList::new();

        for dep in target.deps.iter() {

        }

        CMakeList::new()
    }
}
