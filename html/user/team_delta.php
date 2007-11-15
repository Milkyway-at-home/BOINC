<?php
require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");

$xml = get_int('xml', true);

function show_delta($delta) {
    global $xml;
    $user = BoincUser::lookup_id($delta->userid);
    $when = time_str($delta->timestamp);
    $what = $delta->joining?"joined":"quit";
    if ($xml) {
        echo "    <action>
        <user_email>$user->email_addr</user_email>
        <id>$user->id</id>
        <name>$user->name</name>
        <action>$what</action>
        <total_credit>$delta->total_credit</total_credit>
        <when>$when</when>
    </action>
";
    } else {
        echo "<tr>
           <td>$when</td>
            <td>",user_links($user)," ($user->email_addr)</td>
           <td>$what</td>
           <td>$delta->total_credit</td>
           </tr>
        ";
    }
}

$user = get_logged_in_user();
$teamid = get_int('teamid');
$team = BoincTeam::lookup_id($teamid);
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
}

if (!$team || !is_team_admin($user, $team)) {
    if ($xml) {
        xml_error("-1", "Not admin");
    } else {
        error_page("No admin privileges");
    }
}

if ($xml) {
    echo "<actions>\n";
} else {
    page_head("Team history for $team->name");
    start_table();
    echo "<tr>
        <th>When</th>
        <th>User</th>
        <th>Action</th>
        <th>Total credit at time of action</th>
        </tr>
    ";
}
$deltas = BoincTeamDelta::enum("teamid=$teamid order by timestamp");
foreach($deltas as $delta) {
    show_delta($delta);
}
if ($xml) {
    echo "</actions>\n";
} else {
    end_table();
    page_tail();
}

?>
