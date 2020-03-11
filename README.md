# vsbak

## Description

vsbak - very simple backup - is a simple backup program with command line interface It is based on common Linux tools like `find` and `tar` and offers the possiblity to encrypt the backup archive and to upload the backup on the archive server at LUIS.

The configuration like backup source and destination or information for uploading on the archive server as well as file patterns to exclude from the backup are stored in configuration files.

## Todos

### GUI

Fist of all, the functionality of the command line based script shall be implemented in a C++ program using the Qt framework for an intuitive GUI. The GUI shall provide interfaces for displaying and etiding the configuration ans exclude pattern as well as widgets for displaying the backup source and destination directories. The underlying backup procedure still has to use the Linux toosl which are already used by the commadn line version, like, e.g., `find` and `tar`.

### Restore

Moreover, the GUI shall provide a possibility to select a backup archive for restoring all or singe files into a free chosable destination shall be deisgned and implemented.


## Links

  * [C++](http://www.cplusplus.com/)
    * [C++ Tutorial](http://www.cplusplus.com/doc/tutorial/)
  * [Qt](https://www.qt.io/)
    * [Qt for Beginners](https://wiki.qt.io/Qt_for_Beginners)
    * [Qt Examples And Tutorials](https://doc.qt.io/qt-5/qtexamplesandtutorials.html)
  * [Bash](https://www.gnu.org/software/bash/) (used for the command line version)
    * [Bash scripting cheatsheet](https://devhints.io/bash)

