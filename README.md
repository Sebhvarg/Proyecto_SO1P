# Proyecto_SO1P

curl -SsL https://playit-cloud.github.io/ppa/key.gpg | sudo gpg --dearmor -o /etc/apt/trusted.gpg.d/playit.gpg
echo "deb [signed-by=/etc/apt/trusted.gpg.d/playit.gpg] https://playit-cloud.github.io/ppa/data ./" | sudo tee /etc/apt/sources.list.d/playit.list
sudo apt update
sudo apt install playit
