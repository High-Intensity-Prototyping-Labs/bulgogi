// CMake Module

use crate::target::Target;
use crate::project::Project;
use std::collections::HashMap;

pub struct CMakeProject {
        pub targets: HashMap,
}

impl CMakeProject {
        pub fn new() -> Self {
                CMakeProject {
                        targets: HashMap::new(),
                }
        }
}

impl From<Project> for CMakeProject {
        fn from(project: Project) -> Self {
                for target in project.targets() {
                        
                }
                Project::new()
        }
}

impl From<Target> for (&str, &str) {
       fn from(value: Target) -> Self {
                
       }
}

/**
 *  libmodules.a => any module
 *  libtarget.a => any target (bundle of libmodules)
 *  target.out => any executable
 */
