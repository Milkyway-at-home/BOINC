<?php

require_once("db.inc");
require_once("util.inc");

function nresults($app, $state) {
    $r = mysql_query("select count(*) as nresults from result where appid=$app->id and server_state=$state");
    $foobar = mysql_fetch_object($r);
    mysql_free_result($r);
    return $foobar->nresults;
}

init_session();
db_init();

$platforms = array();
$r2 = mysql_query("select * from platform");
while ($platform = mysql_fetch_object($r2)) {
    if ($platform->deprecated) continue;
    array_push($platforms, $platform);
}
mysql_free_result($r2);

page_head("Applications");
echo "
    <h2>Applications</h2>\n
    ".PROJECT." currently has the following applications.
    When you participate in ".PROJECT.", work for one or
    more of these applications will be assigned to your computer.
    The current version of the application will be downloaded
    to your computer.
    This happens automatically;
    you don't have to do anything.
    <br><br>
";
$result = mysql_query("select * from app");
start_table();


while ($app = mysql_fetch_object($result)) {
    echo "<tr><th colspan=3>$app->user_friendly_name</th></tr>\n";
if (0) {    // this is too inefficient
    $nunsent = nresults($app, 2);
    $ninprogress = nresults($app, 4);
    $ndone = nresults($app, 5);
    echo "<tr><td colspan=3>
        $nunsent results unsent
        <br> $ninprogress results in progress
        <br> $ndone results done
        </td></tr>
    ";
}

    echo "<tr><th>Platform</th><th>Current version</th><th>Installed</th></tr>\n";
    for ($i=0; $i<sizeof($platforms); $i++) {
        $platform = $platforms[$i];
        $newest = null;
        $r2 = mysql_query("select * from app_version where appid=$app->id and platformid = $platform->id");
        while ($av = mysql_fetch_object($r2)) {
            if ($av->deprecated) continue;
            if (!$newest || $av->version_num>$newest->version_num) {
                $newest = $av;
            }
        }
        if ($newest) {
            $x = sprintf("%0.2f", $newest->version_num/100);
            $y = pretty_time_str($newest->create_time);
            echo "<tr>
                <td>$platform->user_friendly_name</td>
                <td>$x</td>
                <td>$y</td>
                </tr>
            ";
        }
    }
}
end_table();
mysql_free_result($result);
page_tail();
?>
