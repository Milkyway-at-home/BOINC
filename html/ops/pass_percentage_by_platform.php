<?php

// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//


require_once("../inc/util_ops.inc");

db_init();
page_head("Pass Percentage by Platform");


$query_appid = $_GET['appid'];
$query_received_time = time() - $_GET['nsecs'];


// First lets get the most recent version numbers per platform
$valid_app_versions = "";

$app_version_query = "
SELECT DISTINCT
       platformid,
       MAX(version_num) AS app_version_num
FROM   app_version
           left join platform on app_version.platformid = platform.id
WHERE
       platform.deprecated <> 1 and
       appid = '$query_appid'
GROUP BY
       platformid;
";

$result = mysql_query($app_version_query);
while ($res = mysql_fetch_object($result)) {
	if (strlen($valid_app_versions) <= 0) {
		$valid_app_versions = "$res->app_version_num";
	} else {
		$valid_app_versions += ", $res->app_version_num";
	}
}
mysql_free_result($result);


// Now that we have a valid list of app_version_nums' lets
//   construct the main query

$main_query = "
SELECT
       DATE_FORMAT(FROM_UNIXTIME(received_time), '%m-%d-%y') AS date,
       app_version_num AS version,
       case
           when INSTR(host.os_name, 'Darwin') then 'Darwin'
           when INSTR(host.os_name, 'Linux') then 'Linux'
           when INSTR(host.os_name, 'Windows') then 'Windows'
           when INSTR(host.os_name, 'SunOS') then 'SunOS'
           else 'Unknown'
       end AS platform,
       COUNT(*) AS total_results,
       ((SUM(case when server_state = '5' and outcome = '1' then 1 else 0 end) / COUNT(*)) * 100) AS pass_rate,
       ((SUM(case when server_state = '5' and outcome = '3' then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate
FROM   result
           left join host on result.hostid = host.id
WHERE
       appid = '$query_appid' and
       server_state = '5' and
       app_version_num IN ( $valid_app_versions ) and
       received_time > '$query_received_time'
GROUP BY
       date,
       version DESC,
       platform;
";

$result = mysql_query($main_query);


echo "<table>\n";
echo "<tr><th>Date</th><th>Version</th><th>Platform</th><th>Total Results</th><th>Pass Rate</th><th>Fail Rate</th></tr>\n";

while ($res = mysql_fetch_object($result)) {

    echo "<tr>";

    echo "<td align=left valign=top>";
    echo $res->date;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->version;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->platform;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->total_results;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->pass_rate;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->fail_rate;
    echo "</td>";

    echo "</tr>\n";

}
mysql_free_result($result);

echo "</table>\n";

page_tail();

?>