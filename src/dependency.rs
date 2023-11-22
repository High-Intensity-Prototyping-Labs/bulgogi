// Dependency Module
pub struct Dependency {
    name: String,
    depkind: DepKind,
}

pub enum DepKind {
    Target,
    Module,
}
