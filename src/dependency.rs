// Dependency Module
pub struct Dependency {
    pub name: String,
    pub depkind: DepKind,
}

pub enum DepKind {
    Target,
    Module,
}
