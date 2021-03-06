// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "defs.h"
#include "SimpleDB.h"
#include "file_system/file_system.hpp"
#include "http/http.hpp"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class SimpleDBHandler : virtual public SimpleDBIf {
private:
  std::vector<int> ports;
  int myPort;

 public:
  SimpleDBHandler(int port) {
    this->myPort = port;

    // appending my port to the ports file
    std::ofstream portsFileOutput;
    portsFileOutput.open(CONTROL_FILENAME, std::ios::app);
    portsFileOutput << port << "\n";
    portsFileOutput.close();

    std::ifstream portsFileInput(CONTROL_FILENAME);
    std::string line;
    if(portsFileInput.is_open())
      {
	while(getline(portsFileInput, line))
	  {
	    ports.push_back(atoi(line.c_str()));
	  }
      }
    portsFileInput.close();

    if(this->ports.size() > 1)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "There are other servers running!" << std::endl;
	std::cout << "I Should tell them about me! :D" << std::endl;
	for(int i = 0; i < this->ports.size() - 1 ; i++) // the last port is me!
	  {
	    shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[i]));
	    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	    SimpleDBClient client(protocol);
	    transport->open();
	    client.update_server();
	    transport->close();
	  }
	std::cout << "Done! :D" << std::endl;

	// avisar aos outros servidores que mais um foi conectado
	// pra isso tem que criar mais um servico no thrift de atualizar de acordo
	// com o arquivo de controle
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "I'm alone :(" << std::endl;
      }

    #if DEBUG
    std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
    std::cout << "Server Created!" << std::endl;
    std::cout << "My Port: " << port << std::endl;
    std::cout << "Ports that I know: " << std::endl;
    for(int i = 0; i < this->ports.size(); i++)
      {
	std::cout << "Port " << i+1 << ": " << this->ports[i] << std::endl;
      }
    #endif
  }

  int apply_hash(const char * s, int size, int count)
  {
    int hash = 0;
    for(int i = 0; i < size; i++)
      {
	hash += (int)s[i];
      }

#if DEBUG
    std::cout << "Applying Hash" << std::endl;
    std::cout << "Value before Hash: " << hash << std::endl;
#endif

    hash = hash % count;

#if DEBUG
    std::cout << "Value after hash: " << hash << std::endl;
#endif

    return hash;
  }

  
  void get_v(File& _return,
	     const std::string& url_v,
	     const std::string& url)
  {
    std::vector<std::string> verified_tokens = Tokenizer::split(url_v.c_str(), '/');
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');

    for(int i = 0 ; i < verified_tokens.size(); i++)
      {
	std::cout << "verified: " << verified_tokens[i] << std::endl;
      }

    for(int i = 0 ; i < tokens.size(); i++)
      {
	std::cout << "url: " << tokens[i] << std::endl;
      }

    /* ----------------------------------------------------
             THINGS ARE ABOUT TO GET REALLY NASTY
       ---------------------------------------------------- */

    if(verified_tokens.size() == 1)
      {
	std::cout << "trabalha normalmente" << std::endl;
	std::cout << "" << std::endl;
	int hash = apply_hash(tokens[verified_tokens.size()].c_str(),
			      tokens[verified_tokens.size()].size(),
			      this->ports.size());
	if(this->ports[hash] != this->myPort)
	  {
	    std::string new_v = url_v + "/" + tokens[verified_tokens.size()];
	    std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	    std::cout << "This is not my work!" << std::endl;
	    std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	    shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	    SimpleDBClient client(protocol);
	    transport->open();
	    client.get_v(_return, new_v, url);
	    transport->close();
	  }
	else {
	  std::string new_v = url_v + "/" + tokens[verified_tokens.size()];
	  this->get_v(_return, new_v, url);
	}
      }
    else
      {
	std::cout << "verifica urls iguais" << std::endl;
	bool ok = true;
	for(int i = 0; i < verified_tokens.size(); i++)
	  {
	    if(verified_tokens[i] != tokens[i])
	      {
		ok = false;
		break;
	      }
	  }

	if(ok)
	  {
	    std::cout << "ok" << std::endl;
	    if(verified_tokens.size() == tokens.size())
	      {
		std::cout << "tamanho igual" << std::endl;
		Node * result = FileSystem::instance()->search(url);

		if(result != NULL)
		  {
		    _return.creation = result->get_creation();
		    _return.modification = result->get_modification();
		    _return.version = result->get_version();
		    _return.name = result->get_name();
		    _return.content = result->get_data();
		  }
	      }
	    else
	      {
		std::cout << "nao tem tamanho igual" << std::endl;
		std::string new_v = url_v + "/" + tokens[verified_tokens.size()];

		
		std::cout << "aplicando hash em: " << tokens[verified_tokens.size()] << std::endl;
		int hash = apply_hash(tokens[verified_tokens.size()].c_str(),
				      tokens[verified_tokens.size()].size(),
				      this->ports.size());
		//std::string new_v = url_v + "/" + tokens[verified_tokens.size()];
		if(this->ports[hash] != this->myPort)
		  {
		    std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
		    std::cout << "This is not my work!" << std::endl;
		    std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
		    shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
		    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		    SimpleDBClient client(protocol);
		    transport->open();
		    client.get_v(_return, new_v, url);
		    transport->close();
		  }
		else
		  {
		    this->get_v(_return, new_v, url);
		  }
	      }
	  }
	else
	  {
	    std::cout << "Nao existe o caminho." << std::endl;
	  }
      }
  }
  
  void get(File& _return, const std::string& url) {
    /*
    this->get_v(_return, "", url);
    return;

    std::cout << "NAO FUNCIONOU" << std::endl;
    */
    
#if DEBUG
    std::cout << "url: " << url << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');

    for(int i = 0; i < tokens.size(); i++)
      {
	std::cout << "token " << i << ": " << tokens[i] << std::endl;
      }
#endif
    
    std::cout << "=========================================" << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());

    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "It's my work!" << std::endl;

	Node * result = FileSystem::instance()->search(url);

	if(result != NULL)
	  {
	    _return.creation = result->get_creation();
	    _return.modification = result->get_modification();
	    _return.version = result->get_version();
	    _return.name = result->get_name();
	    _return.content = result->get_data();
	  }
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	client.get(_return, url);
	transport->close();
      }

    std::cout << "returning: " << _return << std::endl;
    printf("get\n");
  }


  void get_list(std::vector<File> & _return, const std::string& url) {
	Node * result = FileSystem::instance()->search(url);

	if(result == NULL)
	  return;
	
	Node * aux;
	std::vector<File*> files;
	for(int i = 0; i < result->get_child_count(); i++)
	  {
	    aux = result->get_child(i);
	    files.push_back(new File());
	    files[i]->creation = aux->get_creation();
	    files[i]->modification = aux->get_modification();
	    files[i]->version = aux->get_version();
	    files[i]->name = aux->get_name();
	    files[i]->content = aux->get_data();

	    _return.push_back(*(files[i]));
	  }

    printf("get_list\n");
  }
  
  /* ESSA VERSAO DO GET LIST MANDA A REQUISICAO PRA OUTROS SERVIDORES
  void get_list(std::vector<File> & _return, const std::string& url) {
    std::cout << "=========================================" << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());

    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "It's my work!" << std::endl;
	Node * result = FileSystem::instance()->search(url);
	Node * aux;
	std::vector<File*> files;
	for(int i = 0; i < result->get_child_count(); i++)
	  {
	    aux = result->get_child(i);
	    files.push_back(new File());
	    files[i]->creation = aux->get_creation();
	    files[i]->modification = aux->get_modification();
	    files[i]->version = aux->get_version();
	    files[i]->name = aux->get_name();
	    files[i]->content = aux->get_data();

	    _return.push_back(*(files[i]));
	  }
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	client.get_list(_return, url);
	transport->close();
      }

    printf("get_list\n");
  }
  */

  version_t add_tw(const std::string& url, const std::string& content) {
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::vector<std::string> directories;
    std::vector<int> ports_to_send;

    // pegar os diretorios que eu tenho que criar e as portas responsaveis

    if(tokens.size() > 2)
      {	
	File f;
	std::string url2 = "";
	for(int i = 1; i < tokens.size() - 1; i++)
	  {
	    url2 += "/" + tokens[i];
	    File f;
	    this->get(f, url2);
	    if(f.name == "")
	      {
		directories.push_back(url2);
		ports_to_send.push_back(
					this->ports[apply_hash(tokens[i].c_str(),
							       tokens[i].size(),
							       this->ports.size())]
					);
	      }
	  }
      }
    

    for(int i = 0; i < directories.size(); i++)
      {
	std::cout << "vou inserir " << directories[i] << " na porta " << ports_to_send[i] << std::endl;
      }

    if(directories.size() == 1)
      {
	std::cout << "insere direto." << std::endl;
	// eh 1 so insere direto
	int x;
	x = this->add(url, content);
	return x;
      }

    
    // enviar o vote request
    std::vector<bool> respostas;
    std::string msg;
    for(int i = 0; i < ports_to_send.size(); i++)
      {
	std::cout << "Vou enviar um vote request para a porta: "
		  << ports_to_send[i] << std::endl;
	std::string msg = "";

	if(ports_to_send[i] == this->myPort)
	  {
	    std::cout << "Opa! Sou eu!" << std::endl;
	    msg = "Operacao add no diretorio " + directories[i] + "\n";
	    respostas.push_back(this->get_response(msg));
	  }
	else
	  {
	    shared_ptr<TTransport> socket(new TSocket("localhost", ports_to_send[i]));
	    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	    SimpleDBClient client(protocol);
	    transport->open();
	    msg = "Operacao add no diretorio " + directories[i] + "\n";
	    respostas.push_back(client.get_response(msg));
	    //v = client.add(directories[i], content);
	    transport->close();
	  }
	// para cada porta:
	// respostas[i] recebe a resposta do servidor da porta i
	// verdadeiro -> posso efetuar
	// falso -> abort

	// chamar um get response de cada servidor (EM ORDEM)
      }

    //std::cout << "exibindo todas as respostas: " << std::endl;
    bool global = true;
    for(int i = 0; i < respostas.size(); i++)
      {
	global = global && respostas[i];
      }

    if(global)
      {
	std::cout << "Mandando o Global Commit" << std::endl;
	for(int i = 0; i < ports_to_send.size(); i++)
	  {
	    if(ports_to_send[i] == this->myPort)
	      {
		this->tw_feedback("Global Commit!");
		std::cout << "A adicionar o diretorio: " << directories[i] << "no servidor: " << ports_to_send[i] << std::endl;
		this->add(directories[i], ("gerado pela insercao de " + url));
	      }
	    else
	      {
		shared_ptr<TTransport> socket(new TSocket("localhost", ports_to_send[i]));
		shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		SimpleDBClient client(protocol);
		transport->open();
		client.tw_feedback("Global Commit!");
		std::cout << "A adicionar o diretorio: " << directories[i] << "no servidor: " << ports_to_send[i] << std::endl;
		client.add(directories[i], ("gerado pela insercao de " + url));
		transport->close();
	      }
	  }
	
	int x = add(url, content);
	return x;
	// mandar o add pra todo mundo
      }
    else
      {
	std::cout << "Operacao Abortada!" << std::endl;
	for(int i = 0; i < ports_to_send.size(); i++)
	  {
	    if(this->myPort == ports_to_send[i])
	      {
		this->tw_feedback("Operacao Abortada!");
	      }
	    else
	      {
		shared_ptr<TTransport> socket(new TSocket("localhost", ports_to_send[i]));
		shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		SimpleDBClient client(protocol);
		transport->open();
		client.tw_feedback("Operacao Abortada!");
		transport->close();
	      }
	  }
      }

    // percorrer o vetor verificando se e tudo true
    // se for true -> manda add pra todos
    // se for false -> abortar (mandar mensagem de abort)

    
    return 0;
  }

  bool get_response(const std::string& msg) {
    char mander;
    std::cout << "==========================================" << std::endl;
    std::cout << "Mensagem recebida do coordenador!" << std::endl;
    std::cout << "MSG: " << msg << std::endl;
    std::cout << "Commit? (Y/N): ";
    std::cin >> mander;
    std::cout << std::endl;
    std::cout << "==========================================" << std::endl;
    if(mander == 'Y') return true;
    else return false;
  }
  
  void tw_feedback(const std::string& msg) {
    std::cout << "==========================================" << std::endl;
    std::cout << "Mensagem recebida do coordenador!" << std::endl;
    std::cout << "MSG: " << msg << std::endl;
    std::cout << "==========================================" << std::endl;
  }
  
  version_t add(const std::string& url, const std::string& content) {
    std::cout << "Funcao add na url: " << url << " com conteudo: " << content << std::endl;
    
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');

    
    if(tokens.size() > 2)
      {
	File f;
	std::string url2 = "";
	for(int i = 1; i < tokens.size() - 1; i++)
	  {
	    url2 += "/" + tokens[i];
	    std::cout << "estou na verificao da funcao add" << std::endl;
	    std::cout << "url2: " << url2 << std::endl;
	    std::cout << "url2 eh trabalho do: " << apply_hash(tokens[i].c_str(),
							       tokens[i].size(),
							       this->ports.size()) << std::endl;

	    // AQUI EU VOU CONECTAR COM O SERVIDOR RESPONSAVEL PELA URL ATUAL
	    // E DAR UM GET PRA VER SE ELA EXISTE
	    //this->get(f, url2);
	    //	    if (f.name != "")
	    //  {
	    //	std::cout << "existe!" << std::endl;
	    // }
	    //else std::cout << "ele nao existe!" << std::endl;
	    
	    
	    Node * result = FileSystem::instance()->search(url2);
	    if(result == NULL)
	      {
		std::cout << "inserindo: " << url2 << std::endl;
		FileSystem::instance()->insert(url2, "");
	      }
	  }
      }
    

    //return 0;
    std::cout << "=========================================" << std::endl;
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());

    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "It's my work!" << std::endl;
	std::cout << "url:" << url << std::endl;
	Node * result = FileSystem::instance()->insert(url, content);
	/*
	if(result != NULL)
	  {
	    for(int i = 0; i < this->ports.size(); i++)
	      {
		if(this->ports[i] != this->myPort)
		  {
		    shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[i]));
		    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		    SimpleDBClient client(protocol);
		    transport->open();
		    client.add(url, "");
		    transport->close();
		  }
	      }
	      } */
	printf("add\n");
	return result->get_version();
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	int v;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	v = client.add(url, content);
	transport->close();

	return v;
      }

    printf("add\n");
  }

  version_t update(const std::string& url, const std::string& content) {
    std::cout << "=========================================" << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());

    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "Its my work! :D" << std::endl;
	Node * result = FileSystem::instance()->edit(url, content);
	printf("update\n");
	return result->get_version();
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	int v;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	v = client.update(url, content);
	transport->close();
	return v;
      }

    printf("get_list\n");
  }

  void delete_file_tw(File& _return, const std::string& url) {
    Node * result = FileSystem::instance()->search(url);

    // verifica se o nodo tem filhos
    if(result->get_child_count() > 0)
      {
	std::vector<std::string> diretorios_a_remover;
	std::vector<std::string> diretorios_a_verificar;
	std::cout << "Tem mais de um filho!" << std::endl;
	diretorios_a_remover.push_back(url);
	diretorios_a_verificar.push_back(url);
	
	while(diretorios_a_verificar.size() > 0)
	  {
	    std::string diretorio = diretorios_a_verificar[0];
	    diretorios_a_verificar.erase(diretorios_a_verificar.begin());
	    
	    std::vector<File> filhos;

	    for(int i = 0; i < this->ports.size(); i++)
	      {
		std::cout << "quero os filhos de " << diretorio << " que a porta " << this->ports[i] << " conhece." << std::endl;
		if(this->ports[i] == this->myPort)
		  {
		    this->get_list(filhos, diretorio);
		  }
		else
		  {
		    // conectado com o servidor e manda o get list
		    shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[i]));
		    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		    SimpleDBClient client(protocol);
		    transport->open();
		    client.get_list(filhos,diretorio);
		    transport->close();
		  }

			    
		for(int i = 0; i < filhos.size(); i++)
		  {
		    diretorios_a_remover.push_back(diretorio+"/"+filhos[i].name);
		    diretorios_a_verificar.push_back(diretorio+"/"+filhos[i].name);
		  }
		filhos.clear();
	      }
	  }

	std::cout << "Verifiquei!" << std::endl;
	std::cout << "Esses sao os diretories que devem ser deletados: " << std::endl;
	for(int i = 0; i < diretorios_a_remover.size(); i++)
	  {
	    std::cout << diretorios_a_remover[i] << std::endl;
	  }

	// agora comeca o protocolo para remover todos
	// verificando para quais portas devo mandar cada comando
	std::vector<int> ports_to_send;
	for(int i = 0; i < diretorios_a_remover.size(); i++)
	  {
	    std::vector<std::string> tokens = Tokenizer::split(diretorios_a_remover[i].c_str(),
							       '/');
	    ports_to_send.push_back(this->ports[apply_hash(tokens[tokens.size()-1].c_str(),
							   tokens[tokens.size()-1].size(),
							   this->ports.size())]);
	  }

	std::cout << "os comandos para remover devem ser enviados para as portas: " << std::endl;
	for(int i = 0 ; i < ports_to_send.size(); i++)
	  {
	    std::cout << ports_to_send[i] << std::endl;
	  }
	
	// enviando os vote requests
	std::vector<bool> respostas;
	std::string msg;
	for(int i = 0; i < ports_to_send.size(); i++)
	  {
	    std::cout << "Enviando vote request para a porta : "
		      << ports_to_send[i] << std::endl;
	    std::string msg = "";
	    if(ports_to_send[i] == this->myPort)
	      {
		msg = "Operacao de delete no diretorio: " + diretorios_a_remover[i] + "\n";
		respostas.push_back(this->get_response(msg));
	      }
	    else
	      {
		shared_ptr<TTransport> socket(new TSocket("localhost", ports_to_send[i]));
		shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		SimpleDBClient client(protocol);
		transport->open();
		msg = "Operacao de delete no diretorio " + diretorios_a_remover[i] + "\n";
		respostas.push_back(client.get_response(msg));
		
		transport->close();
	      }
	  }

	// enviei os vote requests,a gora tenho que ver as respostas...
	bool global = true;
	for(int i = 0; i < respostas.size(); i++)
	  {
	    global = global && respostas[i];
	  }

	if(global)
	  {
	    // global commit
	    std::cout << "GLOBAL COMMIT" << std::endl;
	    // percorrer os diretorios de tras pra frente e remove-los
	    for(int i = diretorios_a_remover.size() - 1; i >= 0; i--)
	      {
		std::cout << "removendo: " << diretorios_a_remover[i] << std::endl;
		if(ports_to_send[i] == this->myPort)
		  {
		    this->delete_file(_return, diretorios_a_remover[i]);
		  }
		else
		  {
		    shared_ptr<TTransport> socket(new TSocket("localhost", ports_to_send[i]));
		    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		    SimpleDBClient client(protocol);
		    transport->open();
		    client.tw_feedback("Global Commit!");
		    client.delete_file(_return, diretorios_a_remover[i]);
		    
		    transport->close();
		  }
	      }
	  }
	else
	  {
	    // global abort
	    std::cout << "Operacao abortada!" << std::endl;
	    for(int i = 0; i < ports_to_send.size(); i++)
	      {
		if(this->myPort != ports_to_send[i])
		  {
		    shared_ptr<TTransport> socket(new TSocket("localhost", ports_to_send[i]));
		    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		    SimpleDBClient client(protocol);
		    transport->open();
		    client.tw_feedback("Operacao Abortada pelo Coordenador!");
		    transport->close();
		  }
	      }
	  }
      }
    else
      {
	// nao tem filhos
	// so remover
	this->delete_file(_return,url);
      }
    
    printf("delete_file_tw\n");
  }

  void delete_file(File& _return, const std::string& url) {
    std::cout << "=========================================" << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());

    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "Its my work! :D" << std::endl;
	Node * result = FileSystem::instance()->remove(url);

	_return.creation = result->get_creation();
	_return.modification = result->get_modification();
	_return.version = result->get_version();
	_return.name = result->get_name();
	_return.content = result->get_data();
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	client.delete_file(_return, url);
	transport->close();
      }
    printf("delete_file\n");
  }

  version_t update_with_version(const std::string& url, const std::string& content, const version_t version) {
    std::cout << "=========================================" << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());


    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "Its my work! :D" << std::endl;
	Node * s = FileSystem::instance()->search(url);

	if(s->get_version() == version)
	  {
	    Node * result = FileSystem::instance()->edit(url, content);
	    return result->get_version();
	  }
	else
	  {
	    return -1;
	  }
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	int v;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	v = client.update_with_version(url, content, version);
	transport->close();

	return v;
      }

    printf("update_with_version\n");
  }

  void delete_with_version(File& _return, const std::string& url, const version_t version) {
    std::cout << "=========================================" << std::endl;
    std::vector<std::string> tokens = Tokenizer::split(url.c_str(), '/');
    std::string url_to_hash = tokens[tokens.size() - 1];
    int hash = apply_hash(url_to_hash.c_str(), url_to_hash.size(), this->ports.size());

    if(this->ports[hash] == this->myPort)
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "Its my work! :D" << std::endl;
	Node * s = FileSystem::instance()->search(url);

	if(s->get_version() == version)
	  {
	    Node * result = FileSystem::instance()->remove(url);

	    _return.creation = result->get_creation();
	    _return.modification = result->get_modification();
	    _return.version = result->get_version();
	    _return.name = result->get_name();
	    _return.content = result->get_data();
	  }
      }
    else
      {
	std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
	std::cout << "This is not my work!" << std::endl;
	std::cout << "This is " << this->ports[hash] << " job!" << std::endl;
	shared_ptr<TTransport> socket(new TSocket("localhost", this->ports[hash]));
	shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	SimpleDBClient client(protocol);
	transport->open();
	client.delete_with_version(_return, url, version);
	transport->close();
      }
    printf("delete_with_version\n");
  }

    void update_server() {
      std::cout << "=========================================" << std::endl;
      std::cout << "MSG FROM SERVER " << this->myPort << std::endl;
      std::cout << "A new friend joined the server!" << std::endl;
      std::cout << "Let's see who it is!" << std::endl;

      this->ports.clear();
      std::ifstream portsFileInput(CONTROL_FILENAME);
      std::string line;
      if(portsFileInput.is_open())
	{
	  while(getline(portsFileInput, line))
	    {
	      ports.push_back(atoi(line.c_str()));
	    }
	}
      portsFileInput.close();

      std::cout << "Ports that I know: " << std::endl;
      for(int i = 0; i < this->ports.size(); i++)
	{
	  std::cout << "Port " << i+1 << ": " << this->ports[i] << std::endl;
	}
      std::cout << "=========================================" << std::endl;
  }

};

void initialize_server(int port)
{
  shared_ptr<SimpleDBHandler> handler(new SimpleDBHandler(port));
  shared_ptr<TProcessor> processor(new SimpleDBProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

  server.serve();
}

int main(int argc, char **argv) {

  initialize_server(atoi(argv[1]));
  /*
  int thread_count = argc - 1;
  std::thread * db_servers = new std::thread[thread_count];

  int i;
  for(i = 0; i < (thread_count - 1); i++)
    {
      db_servers[i] = std::thread(&initialize_server, atoi(argv[i+1]));
      db_servers[i].detach();

      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "==================" << std::endl;
    }
  initialize_server(atoi(argv[i+1]));
  */

  return 0;
}
