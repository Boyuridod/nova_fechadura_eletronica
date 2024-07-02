package br.com.appchavecompose

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log
import android.widget.Toast
import com.google.firebase.Firebase
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.ValueEventListener
import com.google.firebase.database.database

class FirebaseListenerService : Service() {

    private val database = Firebase.database
    private val campainhaRef = database.getReference("Campainha")

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        campainhaRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val campainhaValue = snapshot.getValue(Int::class.java) ?: 0
                if (campainhaValue == 1) {
                    // Exibir Toast
                    Toast.makeText(this@FirebaseListenerService, "Há alguém na porta!", Toast.LENGTH_SHORT).show()

                    // Resetar o valor para 0
                    campainhaRef.setValue(0)
                }
            }

            override fun onCancelled(error: DatabaseError) {
                Log.e("FirebaseError", "Erro ao ler Campainha: ${error.message}")
            }
        })
        return START_STICKY // Reiniciar o serviço se for interrompido pelo sistema
    }

    override fun onBind(intent: Intent?): IBinder? = null
}
