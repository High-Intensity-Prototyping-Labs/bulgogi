// Dependency Module
pub type Dependency = (String, DepKind);

pub enum DepKind {
    Target,
    Module,
}
