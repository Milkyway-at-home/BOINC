/* Platform independent version definitions... */

#ifndef BOINC_VERSION_H
#define BOINC_VERSION_H

/* Major part of BOINC version number */
#define BOINC_MAJOR_VERSION 5

/* Minor part of BOINC version number */
#define BOINC_MINOR_VERSION 3

/* Release part of BOINC version number */
#define BOINC_RELEASE 16

/* String representation of BOINC version number */
#define BOINC_VERSION_STRING "5.3.16"

#if (defined(_WIN32) || defined(__APPLE__))
/* Name of package */
#define PACKAGE "boinc"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "BOINC"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "BOINC 5.3.16"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "boinc"

/* Define to the version of this package. */
#define PACKAGE_VERSION "5.3.16"

#endif /* #if (defined(_WIN32) || defined(__APPLE__)) */

#endif /* #ifndef BOINC_VERSION_H */

