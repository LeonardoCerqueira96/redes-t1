# redes-t1
# https://github.com/NagisaFawkes/redes-t1

# para compilar servidor:
g++ -o server server.cpp -std=c++11 -pthread

# para compilar cliente:
g++ -o client client.cpp -std=c++11 -pthread

# Usando aplicação: 
	1) execute o servidor em um terminal: "./server [port]"
	2) execute clientes em outro terminais "./client [port] [IP] [nome arbitrário]")
	(Se o servidor for local, IP = 127.0.0.1)
	3) digite mensagens nos terminais dos clientes