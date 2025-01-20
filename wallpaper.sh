ffmpeg -y -framerate 120 -i frame_%04d.bmp -c:v libx264 -pix_fmt yuv420p wallpaper.mp4
mpvpaper HDMI-A-1 /media/code/C/3d/wallpaper.mp4
