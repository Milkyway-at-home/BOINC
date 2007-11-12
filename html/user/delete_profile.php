<?php
require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/profile.inc");

$user = get_logged_in_user();

if (isset($_POST['delete']) && $_POST['delete']) {
    delete_profile($user);
    exit();
}

page_head("Profile delete confirmation");

echo "<form action=", $_SERVER['PHP_SELF'], " method=\"POST\">";

echo "
    <h2>Are you sure?</h2><p>
    Deleted profiles are gone forever and cannot be recovered --
    you will have to start from scratch
    if you want another profile in the future.
    <p>
    If you're sure, click 'Delete'
    to remove your profile from our database.

    <p>
    <input type=submit name=delete value=Delete>
    </form>
";

page_tail();

function delete_profile($user) {
    BoincProfile::delete_aux("userid = $user->id");
    if ($result) {
        delete_user_pictures($user->id);
        page_head("Delete Confirmation");
        $user->update("has_profile=0");
        echo "Your profile has been deleted<br>";
    } else {

        // TODO: Change this to a standard dialog.
        page_head("Deletion Error");
        echo "There was a problem deleting your profile.";
    }
    page_tail();
}

?>
