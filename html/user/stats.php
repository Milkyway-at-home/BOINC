<?php
require_once('db.inc');
require_once('util.inc');
db_init();
page_head('Project statistics');

echo "
<h2>Project statistics</h2>
<p>
<b>Leader boards</b> (information about work done,
both in total and broken down by user, team, and computer)
is not directly available on this web site.
Instead, the raw data is available as compressed XML files.
The format is described
<a href=http://boinc.berkeley.edu/db_dump.php>here</a>,
and the files are
<a href=stats/>here</a>.

<p>
This data can be summarized and represented as Web pages.
An example (in Danish) is at
<a href=http://www.boinc.dk/index.php?page=statistics>http://www.boinc.dk</a>.
";

page_tail();
?>
