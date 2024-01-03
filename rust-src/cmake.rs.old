// CMake module

use crate::project::Project;
use crate::target::Target;
use crate::dependency::{Dependency, DepKind, DepFlag};

/// The largest unit the CMake module understands.
/// Designed to be interchangeable with bulgogi projects.
pub struct CMakeProject {
    pub lists: Vec<CMakeList>
}

/// Grouping of CMakeTargets -- these are better representations of actual compiled targets.
/// (i.e.: every executable (.out) and library (.a) will have a CMakeList of CMakeTargets that are
/// linked together.
pub type CMakeList = Vec<CMakeTarget>;

/// Distinguishes libraries and executables apart.
/// The first string is always the module (dir) name, the second is always the proxy target name.
/// These two will often be the same value in the case of LibModules.
#[derive(Debug)]
pub enum CMakeTarget {
    Library(String),
    LibraryProxy(String, String),
    ExecutableProxy(String, String),
    ExternalLinkage(String),
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            lists: vec![],
        }
    }
}

// impl From<Target> for CMakeList {
//     fn from(target: Target) -> Self {
//         let mut list = CMakeList::new();
// 
//         let module_deps = target.deps.into_iter().filter(|d| d.kind == DepKind::Module).collect::<Vec<Dependency>>();
//         
//         if let Some(exe) = module_deps.clone().into_iter().find(|d| d.flag == DepFlag::Executable) {
//         // Target contains an executable component
//             list.targets.push(CMakeTarget::Executable(exe.name, target.name));
//         } else if let Some(proxy) = module_deps.first() {
//         // First module-dep is proxy 
//             list.targets.push(CMakeTarget::Library(proxy.name.clone(), target.name));
//         } else {
//         // Target contains *no* executable component OR any module deps
//         }
// 
//         list
//     }
// }

impl From<Target> for CMakeList {
    fn from(target: Target) -> Self {
        // Get module deps out of target first
        let module_deps = target.clone().deps.into_iter().filter(|d| matches!(d.kind, DepKind::Module)).collect::<Vec<Dependency>>();

        // Automatically add all non-executable modules as libs 
        let mut list = module_deps.into_iter().filter(|d| matches!(d.flag, DepFlag::None)).map(|d| {
            CMakeTarget::Library(d.name)
        }).collect::<CMakeList>();

        println!("list BEFORE change: {:#?}", list);

        // Add the missing executable component if applicable
        if let Some(exe_component) = target.clone().deps.into_iter().find(|d| matches!(d.flag, DepFlag::Executable)) {
            list.push(CMakeTarget::ExecutableProxy(exe_component.name, target.name));
        } else if let Some(proxy_lib) = list.get_mut(0) {
        // Designate first module lib to be proxy 
            *proxy_lib = CMakeTarget::LibraryProxy(String::from("mememe"), target.name);
        }

        println!("list AFTER change: {:#?}", list);

        // Lastly add all external linkages which were not originally considered modules 
        list.extend(target.deps.into_iter().filter_map(|d| {
            match d.kind {
                DepKind::Target => Some(CMakeTarget::ExternalLinkage(d.name)),
                DepKind::Module => None,
            }
        }));

        println!("list FINALE with ext. linkages: {:#?}", list);

        list
    }
}
