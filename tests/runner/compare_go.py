"""Compare a Go scenario with the same program compiled for the JVM."""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    for tool in ("compiler", "go", "java", "javac", "diff"):
        parser.add_argument(f"--{tool}", required=True)
    return parser.parse_args()


def prepare_workspace(source: Path, work_dir: Path) -> Path:
    work_dir.mkdir(parents=True, exist_ok=True)
    output_dir = work_dir / "output"

    # Prevent stale classes from passing a test after a failed compilation.
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir()

    shutil.copyfile(source, work_dir / "program.go")
    shutil.copyfile(Path(__file__).with_name("go_runtime.go"), work_dir / "go_runtime.go")
    return output_dir


def read_input(source: Path) -> bytes:
    input_file = source.with_suffix(".stdin")
    return input_file.read_bytes() if input_file.exists() else b""


def run_stage(
    work_dir: Path,
    stage: str,
    command: list[str | Path],
    *,
    stdin: bytes = b"",
    timeout: int = 30,
) -> None:
    result = subprocess.run(
        command,
        cwd=work_dir,
        input=stdin,
        capture_output=True,
        timeout=timeout,
    )
    (work_dir / f"{stage}.stdout").write_bytes(result.stdout)
    (work_dir / f"{stage}.stderr").write_bytes(result.stderr)

    if result.returncode != 0:
        stdout = result.stdout.decode(errors="replace")
        stderr = result.stderr.decode(errors="replace")
        raise RuntimeError(f"{stage} exited with {result.returncode}\n{stdout}{stderr}")


def compare_output(work_dir: Path, diff: str) -> None:
    for stream in ("stdout", "stderr"):
        run_stage(
            work_dir,
            f"diff_{stream}",
            [diff, "-u", work_dir / f"go.{stream}", work_dir / f"jvm.{stream}"],
        )


def main() -> None:
    args = parse_args()
    source = args.source.resolve()
    work_dir = args.work_dir.resolve()
    runtime = args.runtime.resolve()
    output_dir = prepare_workspace(source, work_dir)
    stdin = read_input(source)

    run_stage(
        work_dir,
        "go_build",
        [args.go, "build", "-o", "reference", "program.go", "go_runtime.go"],
        timeout=90,
    )
    run_stage(work_dir, "go", [work_dir / "reference"], stdin=stdin)

    run_stage(work_dir, "compile", [args.compiler, source])
    run_stage(work_dir, "javac", [args.javac, "-d", output_dir, runtime])
    run_stage(work_dir, "jvm", [args.java, "-cp", output_dir, "$main"], stdin=stdin)

    compare_output(work_dir, args.diff)
    print(f"{source.name}: Go and JVM output match")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, OSError, subprocess.TimeoutExpired) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
