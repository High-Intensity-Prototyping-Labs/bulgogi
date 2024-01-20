// Root module
mod project;
mod client;
mod template;
mod cmake;

use std::io;

macro_rules! filter_match {
    ($what:expr, $pat:pat, $ret:expr) => {
        match $what {
            $pat => $ret,
            _ => None,
        }
    }
}
pub(crate) use filter_match;

fn main() -> Result<(), io::Error> {
    client::next_command()?;
    Ok(())
}
