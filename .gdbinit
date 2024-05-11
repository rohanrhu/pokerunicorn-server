cd /root/PokerServer
handle SIGPIPE nostop noprint pass
shell make clean
shell make
file build/pkrsrv
set confirm off
set pagination off
set non-stop off
shell tmux split-window
shell echo "tty $(tmux list-panes -F '#{pane_index} #{pane_tty}' | awk -v target=1 '$1 == target {print $2}')" > /tmp/gf-cmd-pane-tty
source /tmp/gf-cmd-pane-tty
python
import os
ssl_key_path = os.getenv("PKRSRV_SSL_KEY_PATH")
ssl_cer_path = os.getenv("PKRSRV_SSL_CER_PATH")
if ssl_key_path and ssl_cer_path:
    print("Using SSL certificate:")
    print("  Key: %s" % ssl_key_path)
    print("  Cer: %s" % ssl_cer_path)
    gdb.execute("set args -sk %s -sc %s" % (ssl_key_path, ssl_cer_path))
print("Using test SSL certificate...")
gdb.execute("r &")
end
set scheduler-locking step