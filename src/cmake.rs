// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module};

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
        // let submodules = project.deps.iter()
        //     .filter_map(|(target_id, dep_list)| filter_match!(project.targets.get(target_id), Some(target), Some(((target_id, target), dep_list))))
        //     .map(|((target_id, target), dep_list)| (CMakeTarget::from((target_id.clone(), target.clone())), dep_list))
        //     .map(|(cmake_target, dep_list)| (cmake_target, dep_list.iter().filter_map(|d| filter_match!(d, Dependency::Target(t), Some(t.clone())))))
        //     .map(|(cmake_target, target_list)| (cmake_target, target_list.collect::<Vec<LibraryName>>()))
        //     .map(|(cmake_target, lib_names)| CMakeList {
        //         target: cmake_target,
        //         links: lib_names,
        //     }).collect::<Vec<CMakeList>>();

        //let submodules = project.deps.iter()
        //    .map(|(target_id, dep_list)| (target_id, dep_list.iter().filter_map(|d| filter_match!(d, Dependency::Module(m), Some(m)))))
        //    .filter_map(|(target_id, proxy)| filter_match!(project.targets.get(target_id), Some(target), Some(((target_id, target), proxy))))
        //    .filter_map(|((target_id, target), mut module_list)| match target {
        //        Target::Library => filter_match!(module_list.nth(0), Some(proxy), Some(((target_id, target), proxy.clone()))),
        //        Target::Executable => filter_match!(module_list.filter_map(|m| filter_match!(project.modules.get(module), Module::Executable, Some(module)))),
        //    })
        //    .filter_map(|((target_id, target), proxy)| filter_match!(project.deps.get(target_id), Some(other_deps), Some(((target_id, target), proxy, other_deps))))
        //    .map(|((target_id, target), proxy, other_deps)| (proxy.clone(), CMakeList {
        //        target: CMakeTarget::from((target_id.clone(), target.clone())),
        //        links: other_deps.iter().filter(|&d| d != &proxy).map(|d| String::from(d.clone())).collect(),
        //    }))
        //    .collect::<HashMap<SubmoduleDir, CMakeList>>();

        // Proxies first, then non proxies
        let targets = project.deps.iter()
            .map(|(target_id, dep_list)| (target_id, dep_list.iter().filter_map(|d| filter_match!(d, Dependency::Module(m), Some(m)))))
            .filter_map(|(target_id, mod_list)| if let Some(target) = project.targets.get(target_id) {
                match target {
                    Target::Executable => filter_match!(mod_list.into_iter().filter_mut()))
                }
            });

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
