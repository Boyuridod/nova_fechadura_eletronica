package br.com.appchavecompose

import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.List
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SmallTopAppBar
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import br.com.appchavecompose.ui.theme.AppChaveComposeTheme
import com.google.firebase.Firebase
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.DatabaseReference
import com.google.firebase.database.ValueEventListener
import com.google.firebase.database.database

val database = Firebase.database
val signingUpRef = database.getReference("SigningUp")

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        val serviceIntent = Intent(this, FirebaseListenerService::class.java)
        startService(serviceIntent)
        setContent {
            AppChaveComposeTheme {
                AppNavigation() // Chame a função de navegação
            }
        }
    }
}
@Composable
fun HomeScreen() {
    // Conteúdo da tela Home (sua VerificationScreen atual)
    VerificationScreen()
}
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChavesCadastradasScreen() {
    var chavesCadastradas by remember { mutableStateOf<Map<String, String>>(emptyMap()) }
    var showErrorToast by remember { mutableStateOf(false) }

    val context = LocalContext.current

    // Referência ao banco de dados Firebase
    val database = Firebase.database
    val chavesRef = database.getReference("ChavesCadastradas")

    // Listener para mudanças nas chaves cadastradas
    LaunchedEffect(chavesRef) {
        chavesRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val chavesMap = snapshot.children.associate { childSnapshot ->
                    childSnapshot.key!! to (childSnapshot.child("nome").getValue(String::class.java) ?: "")
                }
                chavesCadastradas = chavesMap
            }

            override fun onCancelled(error: DatabaseError) {
                Log.e("FirebaseError", "Erro ao ler ChavesCadastradas: ${error.message}")
                showErrorToast = true
            }
        })
    }

    // Exibir Toast de erro se necessário
    if (showErrorToast) {
        Toast.makeText(context, "Erro ao ler dados do Firebase", Toast.LENGTH_SHORT).show()
        showErrorToast = false
    }

    Scaffold(
        topBar = {
            SmallTopAppBar(title = { Text("Chaves Cadastradas") })
        }
    ) { innerPadding ->
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding)
        ) {
            items(chavesCadastradas.keys.toList()) { chaveLida ->
                val nome = chavesCadastradas[chaveLida]
                ChaveItem(nome ?: "", chaveLida, onDelete = {
                    chavesRef.child(chaveLida).removeValue().addOnFailureListener { exception ->
                        Log.e("FirebaseError", "Erro ao apagar chave: ${exception.message}")
                        showErrorToast = true
                    }
                })
            }
        }
    }
}
@Composable
fun ChaveItem(nome: String, codChave: String, onDelete: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(text = "Nome: $nome")
            Text(text = "Código: $codChave")
        }
        IconButton(onClick = {
            onDelete() // Chama o callback para excluir a chave
        }) {
            Icon(Icons.Filled.Delete, contentDescription = "Apagar")
        }
    }
}


@Composable
fun ChaveItem(nome: String, codChave: String, key: String, chavesRef: DatabaseReference, onDelete: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(text = "Nome: $nome")
            Text(text = "Código: $codChave")
        }
        IconButton(onClick = {
            chavesRef.child(key).removeValue() // Remove o nó inteiro usando a chave correta
                .addOnSuccessListener {
                    onDelete() // Chama o callback para atualizar a lista
                }
                .addOnFailureListener { exception ->
                    Log.e("FirebaseError", "Erro ao apagar chave: ${exception.message}")
                    // Lógica para lidar com o erro (ex: exibir um Toast)
                }
        }) {
            Icon(Icons.Filled.Delete, contentDescription = "Apagar")
        }
    }
}


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CadastroChaveScreen(navController: NavController) {
    var nomeChave by remember { mutableStateOf("") }
    var chaveLida by remember { mutableStateOf("") }
    var showErrorToast by remember { mutableStateOf(false) }

    val context = LocalContext.current

    // Referências ao banco de dados Firebase (atualizado para "ChaveRegistrada")
    val database = Firebase.database
    val keyRegisteredRef = database.getReference("ChaveRegistrada") // Correção no nome
    val signingUpRef = database.getReference("SigningUp")
    val chavesCadastradasRef = database.getReference("ChavesCadastradas")
    // Listener para mudanças no valor de KeyRegistered
    LaunchedEffect(keyRegisteredRef) {
        keyRegisteredRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val keyValue = snapshot.getValue(String::class.java)
                chaveLida = keyValue ?: "Insira a chave no leitor"
            }

            override fun onCancelled(error: DatabaseError) {
                Log.e("FirebaseError", "Erro ao ler ChaveRegistrada: ${error.message}")
                showErrorToast = true
            }
        })
    }

    // DisposableEffect para controlar SigningUp
    DisposableEffect(Unit) {
        signingUpRef.setValue(1)
        onDispose {
            signingUpRef.setValue(0)
        }
    }

    // Exibir Toast de erro se necessário
    if (showErrorToast) {
        Toast.makeText(context, "Erro ao ler dados do Firebase", Toast.LENGTH_SHORT).show()
        showErrorToast = false
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        OutlinedTextField(
            value = nomeChave,
            onValueChange = { nomeChave = it },
            label = { Text("Nome da Chave") },
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(modifier = Modifier.height(16.dp))

        OutlinedTextField(
            value = chaveLida,
            onValueChange = { /* Não permitir edição manual */ },
            label = { Text("Chave Lida") },
            modifier = Modifier.fillMaxWidth(),
            readOnly = true
        )

        Spacer(modifier = Modifier.height(24.dp))

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceAround
        ) {
            Button(onClick = {

                val newKeyRef = chavesCadastradasRef.child(chaveLida) //chaveLida

                // Salva os dados da chave
                newKeyRef.setValue(mapOf(
                    "nome" to nomeChave
                )).addOnSuccessListener {
                    // Limpa os campos após o cadastro
                    nomeChave = ""
                    chaveLida = "Insira a chave no leitor"

                    // Limpa o valor de ChaveRegistrada
                    keyRegisteredRef.setValue("")

                    Toast.makeText(context, "Chave cadastrada com sucesso!", Toast.LENGTH_SHORT).show()
                    navController.navigate("chaves")
                }.addOnFailureListener { exception ->
                    Log.e("FirebaseError", "Erro ao cadastrar chave: ${exception.message}")
                    Toast.makeText(context, "Erro ao cadastrar chave.", Toast.LENGTH_SHORT).show()
                }
            }) {
                Text("Cadastrar")
            }

            Button(onClick = {
                navController.navigate("home")
            }) {
                Text("Cancelar")
            }
        }
    }
}


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun VerificationScreen() {
    var inputNumber by remember { mutableStateOf("") }
    var showSwitch by remember { mutableStateOf(false) }
    var switchChecked by remember { mutableStateOf(false) } // Inicia como desligado

    val database = Firebase.database
    val switchRef = database.getReference("abrePorta")

    LaunchedEffect(switchRef) {
        switchRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val isOpenValue = snapshot.getValue(Int::class.java) ?: 0
                switchChecked = isOpenValue == 1
            }

            override fun onCancelled(error: DatabaseError) {
                // Tratar erros aqui (por exemplo, exibir um Toast)
            }
        })
    }

    Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        OutlinedTextField(
            value = inputNumber,
            onValueChange = { inputNumber = it },
            label = { Text("Digite o número") },
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
        )

        Spacer(modifier = Modifier.height(16.dp))

        Button(
            onClick = {
                if (inputNumber == "123456") {
                    showSwitch = true
                } else {
                    // Feedback de erro aqui
                }
            }
        ) {
            Text("Confirmar")
        }

        Spacer(modifier = Modifier.height(16.dp))

        if (showSwitch) {
            Row(
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text("Aberta")
                Spacer(modifier = Modifier.width(8.dp))
                Switch(
                    checked = switchChecked,
                    onCheckedChange = { newValue ->
                        switchChecked = newValue
                        switchRef.setValue(if (newValue) 1 else 0)
                    }
                )
            }
        }
    }
}


@Composable
fun AppNavigation() {
    val navController = rememberNavController() // Cria o controlador de navegação

    Scaffold(
        bottomBar = {
            NavigationBar {
                NavigationBarItem(
                    icon = { Icon(Icons.Filled.Home, contentDescription = "Home") },
                    label = { Text("Home") },
                    selected = currentRoute(navController) == "home",
                    onClick = { navController.navigate("home") }
                )
                NavigationBarItem(
                    icon = { Icon(Icons.Filled.Add, contentDescription = "Cadastro") },
                    label = { Text("Cadastro") },
                    selected = currentRoute(navController) == "cadastro",
                    onClick = { navController.navigate("cadastro") }
                )
                NavigationBarItem(
                    icon = { Icon(Icons.Filled.List, contentDescription = "Chaves") },
                    label = { Text("Chaves") },
                    selected = currentRoute(navController) == "chaves",
                    onClick = { navController.navigate("chaves") }
                )
            }
        }
    ) { innerPadding ->
        NavHost(navController = navController, startDestination = "home", modifier = Modifier.padding(innerPadding)) {
            composable("home") { HomeScreen() }
            composable("cadastro") { CadastroChaveScreen(navController) }
            composable("chaves") { ChavesCadastradasScreen() }
        }
    }
}

// Função auxiliar para obter a rota atual
@Composable
fun currentRoute(navController: NavController): String? {
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    return navBackStackEntry?.destination?.route
}

