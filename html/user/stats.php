<?php
require_once('../inc/db.inc');
require_once('../inc/util.inc');
db_init();
page_head('Project statistics');

echo "
<p>
Data describing the users, teams, and computers
participating in this project,
and how much work each has done,
is available as compressed XML files.
The format is described
<a href=http://boinc.berkeley.edu/db_dump.php>here</a>,
and the files are
<a href=stats/>here</a>.

<p>
This data can be summarized and represented as Web pages.
Examples:
<ul>
<li>
<a href=http://www.boinc.dk/index.php?page=statistics>http://www.boinc.dk</a>,
developed by <a href=mailto:stats@boinc.dk>Janus Kristensen</a>.
<li>
<a href=http://www.saschapfalz.de/boincstats/boinc-stats.php>boincstats</a>,
developed by Sascha Pfalz.
<li>
<a href=http://stats.boincbzh.net/BZHwds/index.php>BOINC Alliance Francophone</a>,
developed by Vincent Mary (email: stats at hoincbzh dot net).
Supports competition between 'mini-teams'.
</ul>
These systems are implemented using PHP,
and the source code may be available.
If you are interested in running your own site or
participating in the development efforts,
please contact the people listed above.
";

page_tail();
?>
