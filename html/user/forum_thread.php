<?php
$cvs_version_tracker[]="\$Id$";
/**
 * This file displays the contents of a thread.
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
db_init();

$threadid = get_int('id');
$sort_style = get_int('sort', true);
$filter = get_str('filter', true);

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

$logged_in_user = re_get_logged_in_user(false);
if ($logged_in_user) {
    $tokens = url_tokens($logged_in_user->getAuthenticator());
}

// Fetch the thread and increment the number of views
$thread = new Thread($threadid);

if ($thread->isHidden()) {
    if ((!$logged_in_user) || (($logged_in_user) && (!$logged_in_user->isSpecialUser(S_MODERATOR)))) {
        /* If the user logged in is a moderator, show him the
        * thread if he goes so far as to name it by ID like this.
        * Otherwise, hide the thread.
        */
        error_page(tr(FORUM_THREAD_HIDDEN));
    }
}

$forum = $thread->getForum();
$category = $forum->getCategory();

$title = cleanup_title($thread->getTitle());
if (!$sort_style) {
    // get the sorting style from the user or a cookie
    if ($logged_in_user){
        $sort_style = $logged_in_user->getThreadSortStyle();
    } else {
        list($forum_style, $sort_style)=explode("|",$_COOKIE['sorting']);
    }
} else {
    if ($logged_in_user){
        $logged_in_user->setThreadSortStyle($sort_style);
    } else {
        list($forum_style,$old_style)=explode("|",$_COOKIE['sorting']);
        setcookie('sorting', implode("|",array($forum_style,$sort_style)), time()+3600*24*365);
    }
}



if ($logged_in_user && $logged_in_user->hasJumpToUnread()){
    page_head($title, 'jumpToUnread();');
    echo "<link href=\"forum_forum.php?id=".$forum->getID()."\" rel=\"up\" title=\"".$forum->getTitle()."\">";
} else {
    page_head($title);
    echo "<link href=\"forum_forum.php?id=".$forum->getID()."\" rel=\"up\" title=\"".$forum->getTitle()."\">";
}

$is_subscribed = $logged_in_user && $thread->isSubscribed($logged_in_user);

show_forum_title($forum, $thread);

if ($thread->getType()!=0 && $thread->getStatus()==0){
    $thread_owner = $thread->getOwner();
    if ($logged_in_user){
            if ($thread_owner->getID() == $logged_in_user->getID()){
                if ($thread->getPostCount()!=0) {
                    // Show a "this question has been answered" to the author
                    echo "<div class=\"helpdesk_note\">
                    <form action=\"forum_thread_status.php\"><input type=\"hidden\" name=\"id\" value=\"".$thread->getID()."\">
                    <input type=\"submit\" value=\"My question was answered\">
                    </form>
                    If your question has been adequately answered please click here to close it!
                    </div>";
                }
            } else {
                // and a "I also got this question" to everyone else if they havent already told so
                echo "<div class=\"helpdesk_note\">
                <form action=\"forum_thread_vote.php\"><input type=\"hidden\" name=\"id\" value=\"".$thread->getID()."\">
                <input type=\"submit\" value=\"I've also got this question\">
                </form>
                </div>";
            }
        }
    }
    
    echo "
        <form action=\"forum_thread.php\">
        <input type=\"hidden\" name=\"id\" value=\"", $thread->getID(), "\">
        <table width=\"100%\" cellspacing=0 cellpadding=0>
        <tr>
        <td align=\"left\">
    ";
    
    $reply_url = "";
    if (can_reply($thread, $logged_in_user)) {        
        $reply_url = "forum_reply.php?thread=".$thread->getID()."#input";
        show_button($reply_url, tr(FORUM_THREAD_REPLY), "Add a new message to this thread");
    }
    
    if ($is_subscribed) {
        $url = "forum_subscribe.php?action=unsubscribe&thread=".$thread->getID()."$tokens";
        show_button($url, tr(FORUM_THREAD_UNSUBSCRIBE), "You are subscribed to this thread.  Click here to unsubscribe.");
    } else {
        $url = "forum_subscribe.php?action=subscribe&thread=".$thread->getID()."$tokens";
        show_button($url, tr(FORUM_THREAD_SUBSCRIBE), "Click to get email when there are new posts in this thread");
    }
    
    //If the logged in user is moderator enable some extra features
    if ($logged_in_user && $logged_in_user->isSpecialUser(S_MODERATOR)){
        if ($thread->isHidden()){
            echo "<br /><a href=\"forum_moderate_thread_action.php?action=unhide&amp;thread=".$thread->getID()."$tokens\">Un-Delete this thread</a>";
        } else {
            echo "<br /><a href=\"forum_moderate_thread.php?action=hide&amp;thread=".$thread->getID()."\">Delete this thread</a>";
        }
        if ($thread->isSticky()){
            echo "<br /><a href=\"forum_moderate_thread_action.php?action=desticky&amp;thread=".$thread->getID()."$tokens\">De-sticky this thread</a>"; 
        } else {
            echo "<br /><a href=\"forum_moderate_thread_action.php?action=sticky&amp;thread=".$thread->getID()."$tokens\">Make this thread sticky</a>";
        }
        if ($thread->isLocked()) {
            echo "<br /><a href=\"forum_moderate_thread_action.php?action=unlock&amp;thread=".$thread->getID()."$tokens\">Unlock thread</a>";
        } else {
            echo "<br /><a href=\"forum_moderate_thread_action.php?action=lock&amp;thread=".$thread->getID()."$tokens\">Lock thread</a>";
        }
        echo "<br /><a href=\"forum_moderate_thread.php?action=move&amp;thread=".$thread->getID()."$tokens\">Move this thread</a>";
        echo "<br /><a href=\"forum_moderate_thread.php?action=title&amp;thread=".$thread->getID()."$tokens\">Edit thread title</a>";
    }

    // Display a box that allows the user to select sorting of the posts
    echo "</td><td align=right style=\"border:0px\">";
    echo "Sort ";
    echo select_from_array("sort", $thread_sort_styles, $sort_style);
    echo "<input type=submit value=OK>\n</td>";
    echo "</tr>\n</table>\n</form>\n";

    // Here is where the actual thread begins.
    $headings = array(array(tr(FORUM_AUTHOR),"authorcol"), array(tr(FORUM_MESSAGE),"",2));

    start_forum_table($headings, "id=\"thread\" width=100%");
    show_posts($thread, $sort_style, $filter, $logged_in_user, true);
    end_forum_table();

    if ($reply_url) {
        show_button($reply_url, tr(FORUM_THREAD_REPLY), "Add a new message to this thread");
    }
    show_forum_title($forum, $thread, false);
    $thread->incViews();

page_tail();
?>
