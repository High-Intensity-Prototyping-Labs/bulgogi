// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency};

use std::collections::HashMap;

pub type LibraryName = String;
pub type ExecutableName = String;

pub struct CMakeProject {
    pub submodules: Vec<CMakeList>,
}

pub struct CMakeList {
    pub target: CMakeTarget,
    pub links: Vec<LibraryName>,
}

pub enum CMakeTarget {
    Library(LibraryName),
    Executable(ExecutableName),
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            submodules: Vec::new(),
        }
    }
}

impl From<Project> for CMakeProject {
    fn from(project: Project) -> Self {
        let submodules = project.deps.iter()
            .filter_map(|(target_id, dep_list)| filter_match!(project.targets.get(target_id), Some(target), Some(((target_id, target), dep_list))))
            .map(|((target_id, target), dep_list)| (CMakeTarget::from((target_id.clone(), target.clone())), dep_list))
            .map(|(cmake_target, dep_list)| (cmake_target, dep_list.iter().filter_map(|d| filter_match!(d, Dependency::Target(t), Some(t.clone())))))
            .map(|(cmake_target, target_list)| (cmake_target, target_list.collect::<Vec<LibraryName>>()))
            .map(|(cmake_target, lib_names)| CMakeList {
                target: cmake_target,
                links: lib_names,
            }).collect::<Vec<CMakeList>>();

        CMakeProject {
            submodules,
        }
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
