<?php

    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

function send_verify_email($user, $email_addr, $key) {
    mail(
        $email_addr,
        PROJECT." account email change",
"You have asked that the email address of your PROJECT account
be changed to $email_addr.
To confirm this change, please visit the following URL:
".MASTER_URL."/confirm_email_change.php?id=$user->id&str=$key"
    );
}

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    require_login($user);

    $email_addr = $HTTP_POST_VARS["email_addr"];

    page_head("Edit email address");
    if ($email_addr == "Verification pending") {
        echo "You previously requested an email address change.
            An email was sent to the new address,
            containing a URL that you must visit to verify the new address.
        ";
    } else if (!is_valid_email_addr($email_addr)) {
        echo "Invalid email address requested";
    } else if ($email_addr == $user->email_addr) {
        echo "No change requested";
    } else {
        $existing = null;
        $result = mysql_query("select * from user where email_addr='$email_addr'");
        if ($result) {
            $existing = mysql_fetch_object($result);
            mysql_free_result($result);
        }
        if ($existing) {
            echo "There's already an account with that email address";
        } else {
            $x = random_string();
            $y = munge_email_addr($email_addr, $x);
            $result = mysql_query("update user set email_addr='$y' where id=$user->id");
            if ($result) {
                send_verify_email($user, $email_addr, $x);
                echo "
                    An email has been sent to $email_addr,
                    containing a URL that you must visit to verify the address.
                ";
            } else {
                echo "
                    We can't update your email address
                    due to a database problem.  Please try again later.
                ";
            }
        }
    }

    page_tail();

?>
