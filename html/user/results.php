<?php
// show recent results for a host or user

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");

$config = get_config();
if (!parse_bool($config, "show_results")) {
    error_page("This feature is turned off temporarily");
}

$results_per_page = 20;

db_init();
$hostid = get_int("hostid", true);
$userid = get_int("userid", true);
$offset = get_int("offset", true);
if (!$offset) $offset=0;

if ($hostid) {
    $host = lookup_host($hostid);
    $type = "computer";
    $clause = "hostid=$hostid";
} else {
    $user = get_logged_in_user();
    if ($userid != $user->id) {
        error_page("No access");
    }
    $type = "user";
    $clause = "userid=$userid";
}
page_head("Tasks for $type");
result_table_start(true, false, true);
$i = 0;
$query = "select * from result where $clause order by id desc limit $offset,".($results_per_page+1);
$result = mysql_query($query);
$number_of_results = mysql_affected_rows();
echo show_result_navigation(
    $clause, $number_of_results, $offset, $results_per_page
);
while ($res = mysql_fetch_object($result) and $i<$results_per_page) {
    show_result_row($res, true, false, true);
    $i++;
}
mysql_free_result($result);
echo "</table>\n";

echo show_result_navigation(
    $clause, $number_of_results, $offset, $results_per_page
);

page_tail();
?>
