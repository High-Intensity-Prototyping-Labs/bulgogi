// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module, ModuleID, ProjectChild};

use std::collections::HashMap;

pub type SubmoduleDir = String;
pub type LibraryName = String;
pub type ExecutableName = String;

#[derive(Debug)]
pub struct CMakeProject {
    pub submodules: HashMap<SubmoduleDir, CMakeList>,
}

#[derive(Debug)]
pub struct CMakeList {
    pub target: CMakeTarget,
    pub links: Vec<LibraryName>,
}

#[derive(Debug)]
pub enum CMakeTarget {
    Library(LibraryName),
    Executable(ExecutableName),
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            submodules: HashMap::new(),
        }
    }
}

impl From<Project> for CMakeProject {
    fn from(project: Project) -> Self {
        let proxy_libs = project.deps.iter()
            .map(|(target_id, dep_list)| (target_id, dep_list.module_ids()))
            .filter_map(|(target_id, mod_list)| filter_match!(mod_list.first(), Some(m), Some((target_id.clone(), m.clone()))))
            .collect::<HashMap<TargetID, SubmoduleDir>>();

        //CMakeProject {
        //    submodules,
        //}
        CMakeProject::new()
    }
}

impl From<(TargetID, Target)> for CMakeTarget {
    fn from(entry: (TargetID, Target)) -> Self {
        match entry.1 {
            Target::Library => CMakeTarget::Library(entry.0),
            Target::Executable => CMakeTarget::Executable(entry.0),
        }
    }
}
