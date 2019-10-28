## vsbak (command line interface)

This folder contains the command line version of vsbak together with the required configuration files 

### Files and their required location

| File         | Function                               | Location          |
|:-------------|:---------------------------------------|:------------------|
| vsbak        | Shell script                           | `~/.local/bin`    |
| main.conf    | Configuration file                     |` ~/.config/vsbak` |
| exclude.conf | List of pattern to exclude from backup | `~/.config/vsbak` |

### Configuration

#### main.conf

The main configuration files follows the bash syntax for defining shell variables. The following variables must or can be configured.

| Variable   | Meaning                                            | Remarks  |
|:-----------|:---------------------------------------------------|-----:----|
| `bak_src`  | Directory to be backed up                          | Mandatory|
| `bak_dest` | Destination directory of the backup archive        | Mandatory|
| `gpg_key`  | Key from gnupg keyring used for encryption         | Optional |
| `arc_user` | User on archiv.luis.uni-hannover.de for uploading  | Optional |
| `arc_path` | Path on archiv.luis.uni-hannover.de for uploading  | Optional |


  * If `key` is not present, symmetric encryption with a passphrase is used instead of a public key.
  * If `arc_user` is not present, the backup archive is not uploaded
  * If `arc_path` is not present, the backup archive is uploaded into the root folder of the server


#### exclude.conf

This file contains file or directory patterns that should be excluded from the backup, e.g.,

| Pattern          | Meaning                                         |
|:-----------------|:------------------------------------------------|
| `*.iso`          | Exclude all iso-images                          |
| `.trash/**`      | Exclude all files in the folder `.trash`        |
| `*-git/**`       | Exclude all files in folders ending with `-git` |
| `./Downloads/**` | Exclude all files in the download folder        |




