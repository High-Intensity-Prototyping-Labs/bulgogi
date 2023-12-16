// Project module 

use std::collections::hash_map;
use std::collections::HashMap;

type TargetID = String;
type ModuleDir = String;

enum Dependency {
    Module(ModuleDir),
    Target(TargetID),
}

struct Project {
    layout: HashMap<TargetID, Vec<Dependency>>,
}

impl Project {
    pub fn new() -> Self {
        Project {
            layout: HashMap::new(),
        }
    }

    pub fn targets(&self) -> hash_map::Iter<'_, String, Vec<Dependency>> {
        self.layout.iter()
    }

    pub fn modules(&self) -> impl Iterator<Item=&Dependency> {
        self.layout.values().flat_map(|l| l.into_iter().filter(|d| matches!(d, Dependency::Module(_))))
    }
}
