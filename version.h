/* Platform independent version definitions... */

#ifndef BOINC_VERSION_H
#define BOINC_VERSION_H

/* Major part of BOINC version number */
#define BOINC_MAJOR_VERSION 4

/* Minor part of BOINC version number */
#define BOINC_MINOR_VERSION 30

/* String representation of BOINC version number */
#define BOINC_VERSION_STRING "4.30"

#ifdef _WIN32
/* Name of package */
#define PACKAGE "boinc"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "BOINC"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "BOINC 4.30"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "boinc"

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.30"

#endif /* #ifdef _WIN32 */

#endif /* #ifndef BOINC_VERSION_H */

