#!/bin/sh

FTP_GROUP=ftpgroup

addgroup -S $FTP_GROUP
adduser -S $FTP_USER -G ftpgroup -h /home/$FTP_USER/
chown $FTP_USER:$FTP_GROUP /home/$FTP_USER/ -R

echo "$FTP_USER:$FTP_PASS" | /usr/sbin/chpasswd

cat <<- EOF >> /tmp/vsftpd.conf

listen=YES
connect_from_port_20=YES
port_enable=YES
pasv_enable=YES
pasv_max_port=10100
pasv_min_port=10090
pasv_address=127.0.0.1

write_enable=YES
dirmessage_enable=YES
allow_writeable_chroot=yes

anonymous_enable=NO
local_enable=YES
chroot_local_user=YES
passwd_chroot_enable=YES
allow_writeable_chroot=YES

local_umask=022

seccomp_sandbox=NO

vsftpd_log_file=/tmp/vsftpd.log
log_ftp_protocol=YES
ftpd_banner=Welcome to FTP Server for Xpiks test

EOF

cp /tmp/vsftpd.conf /etc/vsftpd/vsftpd.conf

/usr/sbin/vsftpd /etc/vsftpd/vsftpd.conf
