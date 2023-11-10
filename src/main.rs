use std::ffi::OsString;

use clap::{arg, Command};

fn cli() -> Command {
    Command::new("bulgogi")
        .about("A build system orchestrator for CubeSat projects")
        .subcommand_required(true)
        .arg_required_else_help(true)
        .allow_external_subcommands(true)
        .subcommand(
            Command::new("init")
                .about("Initializes bulgogi project directory")
                .arg(arg!(<PATH> "The directory to initialize"))
                .arg_required_else_help(true),
        )
        .subcommand(
            Command::new("module")
            .about("Manage project modules")
            .arg(arg!(<COMMAND> "What to do with the module"))
            .arg(arg!(<PATH> "Path to the module in question"))
            .arg(arg!(--create))
        )
        .subcommand(
            Command::new("build")
            .about("Builds the bulgogi project")
        )
        .subcommand(
            Command::new("clean")
            .about("Cleans the local project build files and cache")
        )
        .subcommand(
            Command::new("rm")
            .alias("remove")
            .about("Removes a module from the bulgogi project")
            .arg(arg!(<PATH> "Path to the module to remove"))
            .arg(arg!(--cached))
        )
}

fn main() {
    let matches = cli().get_matches();

    match matches.subcommand() {
        Some(("init", sub_matches)) => {
            println!(
                "Initialized bulgogi project {}",
                sub_matches.get_one::<String>("PATH").expect("required")
            );
        }
        Some(("module", sub_matches)) => {
            let create = sub_matches.get_flag("create");

            println!(
                "Doing {} to module {} (--create = {})",
                sub_matches.get_one::<String>("COMMAND").expect("required"),
                sub_matches.get_one::<String>("PATH").expect("required"),
                create,
            );
        }
        Some(("build", _)) => {
            println!(
                "Building bulgogi project..."
            );
        }
        Some(("clean", _)) => {
            println!(
                "Cleaning bulgogi project..."
            );
        }
        Some(("rm", sub_matches)) => {
            let cached = sub_matches.get_flag("cached");

            println!(
                "Remove project module {} (--cached = {})",
                sub_matches.get_one::<String>("PATH").expect("required"),
                cached,
            );
        }
        Some((ext, sub_matches)) => {
            let args = sub_matches
                .get_many::<OsString>("")
                .into_iter()
                .flatten()
                .collect::<Vec<_>>();
            println!("Calling out to {ext:?} with {args:?}");
        }
        _ => unreachable!(),
    }

    println!("Hello, world!");
}
