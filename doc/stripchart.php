<?php
require_once("docutil.php");
page_head("Stripcharts");
echo "

<h3>Goals</h3>
<p>
Stripcharts is a system for recording and displaying
time-varying system load and performance information.
The basic idea:

<ul>
<li> A set of scripts, run periodically using crontab,
record performance metrics
(CPU load, network connection rates, queue lengths, etc.)
as time-stamped entries in log files.
<li> A web interface lets you generate graphs of
combinations of these quantities,
over whatever time scale you want.
</ul>
Looking at these graphs can be a major help in tracking
down problems in complex server systems.

<p>
Stripcharts is independent of BOINC,
but it's especially helpful for diagnosing performance
problems in BOINC servers.
";
page_tail();
?>
