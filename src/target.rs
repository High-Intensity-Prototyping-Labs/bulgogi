// Target module
pub struct Target {
    name: String,
    deps: Vec<(String,DepKind)>,
}

pub enum DepKind {
    Target,
    Module,
}

