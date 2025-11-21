import re
import sys

def bump_version(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Regex to match project(VST3-Open-Valhalla VERSION X.Y.Z)
    pattern = r'project\((VST3-Open-Valhalla\s+VERSION\s+)(\d+)\.(\d+)\.(\d+)\)'

    match = re.search(pattern, content)
    if not match:
        print("Error: Version pattern not found in CMakeLists.txt")
        sys.exit(1)

    prefix = match.group(1)
    major = int(match.group(2))
    minor = int(match.group(3))
    patch = int(match.group(4))

    # Increment patch version
    new_patch = patch + 1
    new_version = f"{major}.{minor}.{new_patch}"

    new_content = re.sub(pattern, f'project({prefix.strip()} {new_version})', content)

    with open(file_path, 'w') as f:
        f.write(new_content)

    print(new_version)

if __name__ == "__main__":
    bump_version('CMakeLists.txt')
