```c
ssh-keygen -t ed25519 -C "your_email@example.com"
cat ~/.ssh/id_ed25519.pub
// git hub에 해당 ssh 코드 등록 후 
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519

ssh -T git@github.com
```