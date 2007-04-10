(defun my-save-and-compile ()
  (interactive "")
  (save-buffer 0)
  (compile "make -j2 -C ~/files/build/libs/lame -k"))


(defun my-save-and-compile ()
  (interactive "")
  (save-buffer 0)
  (compile "make -j2 -C ~/files/build/libs/streaming -k"))


(defun my-save-and-compile ()
  (interactive "")
  (save-buffer 0)
  (compile "make -j2 -C ~/files/build/test -k"))

(defun my-save-and-compile ()
  (interactive "")
  (save-buffer 0)
  (compile "make -k -j2 -C ~/code/build"))

