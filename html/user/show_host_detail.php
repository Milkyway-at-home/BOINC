<?php
    require_once("util.inc");
    require_once("db.inc");
    require_once("user.inc");
    require_once("host.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    $hostid = $HTTP_GET_VARS["hostid"];
    if ($user && $hostid) {
        page_head("Host stats");

        $result = mysql_query("select * from host where id = $hostid");
        $host = mysql_fetch_object($result);
        mysql_free_result($result);

        if ($host) {
            if ($host->userid != $user->id) {
                echo "Not your host\n";
            } else {
                show_host($host, true);
            }
        } else {
            echo "Couldn't find host or user.<p>";
        }
        page_tail();
    } else {
        print_login_form();
    }
?>
