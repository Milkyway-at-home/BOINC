<?php

    require_once("db.inc");
    require_once("util.inc");
    require_once("team.inc");

    db_init();
    $user = get_logged_in_user();
    $teamid = $_GET["teamid"];

    $result = mysql_query("select * from team where id = $teamid");
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    require_founder_login($user, $team);
    $team_name = $team->name;
    $team_id = $team->id;
    $nusers = $team->nusers;
    page_head("Remove Members from $team_name");
    echo "
        <h2>Remove members from $team_name</h2>
        Removing a member will subtract their credit from team totals
        <form method=post action=team_remove_inactive_action.php>
        <input type=hidden name=id value=$team_id>
    ";
    start_table();
    echo "<tr>
        <th>Remove?</th>
        <th>Name</th>
.        <th>Total credit</th>
        <th>Recent average credit</th>
        </tr>
    ";

    $result = mysql_query("select * from user where teamid = $team_id");

    $ninactive_users = 0;
    while ($user = mysql_fetch_object($result)) {
        $user_total_credit = format_credit($user->total_credit);
        $user_expavg_credit = format_credit($user->expavg_credit);
        echo "
            <tr>
            <td align=center><input type=checkbox name=remove_$ninactive_users value=$user->id>
            <td>$user->name</td>
            <td>$user_total_credit</td>
            <td>$user_expavg_credit</td>
        ";
        $ninactive_users++;
    }
    echo "<input type=hidden name=ninactive_users value=$ninactive_users>";
    if ($result) {
       mysql_free_result($result);
    }
    end_table();
    echo "<input type=submit value=\"Remove users\">";
    echo "</form>";
    page_tail();
?>
