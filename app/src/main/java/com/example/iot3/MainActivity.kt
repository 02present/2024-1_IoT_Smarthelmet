package com.example.iot3

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.util.Log
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
//import com.example.iot3.MainActivity.ConnectedThread.Companion.MESSAGE_READ
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

class MainActivity : AppCompatActivity() {
    private val TAG = "BluetoothArduino"
    private val REQUEST_ENABLE_BT = 1
    private val REQUEST_FINE_LOCATION = 2
    private val DEVICE_ADDRESS = "00:00:00:00:00:00"
    private val MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private lateinit var bluetoothAdapter: BluetoothAdapter
    private var bluetoothSocket: BluetoothSocket? = null
    private var connectedThread: ConnectedThread? = null
    private lateinit var handler: Handler
    private lateinit var speed: TextView
    private lateinit var bright: TextView
    private lateinit var connectButton: Button

    companion object {
        const val MESSAGE_READ = 0
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        speed = findViewById(R.id.tv_BH1750)
        bright = findViewById(R.id.tv_MPU9250)
        connectButton = findViewById(R.id.connectbutton)

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth not supported", Toast.LENGTH_SHORT).show()
            finish()
            return
        }

        connectButton.setOnClickListener {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), REQUEST_FINE_LOCATION)
            } else {
                connectToDevice()
            }
        }
        handler = Handler(Handler.Callback { msg ->
            if (msg.what == MESSAGE_READ) {
                val readMessage = msg.obj as String
                processBluetoothMessage(readMessage)
            }
            true
        })
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            REQUEST_FINE_LOCATION -> {
                if ((grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    connectToDevice()
                } else {
                    Toast.makeText(this, "Location permission is required for Bluetooth connection", Toast.LENGTH_SHORT).show()
                }
                return
            }
        }
    }

    private fun connectToDevice() {
        // 블루투스 장치 가져오기
        val device: BluetoothDevice = bluetoothAdapter?.getRemoteDevice(DEVICE_ADDRESS) ?: return

        // 블루투스 권한 확인
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED) {
            // 권한이 없는 경우 권한 요청
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.BLUETOOTH, Manifest.permission.BLUETOOTH_ADMIN), REQUEST_ENABLE_BT)
        } else {
            // 권한이 있는 경우 소켓 연결 시도
            try {
                bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID)
                bluetoothSocket?.connect()
                connectedThread = ConnectedThread(bluetoothSocket!!)
                connectedThread?.start()
                Toast.makeText(this, "Connected to device", Toast.LENGTH_SHORT).show()
            } catch (e: IOException) {
                Log.e(TAG, "Error connecting to device", e)
                Toast.makeText(this, "Connection failed", Toast.LENGTH_SHORT).show()
            }
        }
    }


    private inner class ConnectedThread(private val socket: BluetoothSocket) : Thread() {
        private val inStream: InputStream = socket.inputStream
        private val outStream: OutputStream = socket.outputStream

        override fun run() {
            val buffer = ByteArray(1024)
            var bytes: Int

            while (true) {
                try {
                    bytes = inStream.read(buffer)
                    val readMessage = String(buffer, 0, bytes)
                    handler.obtainMessage(MESSAGE_READ, readMessage).sendToTarget()
                } catch (e: IOException) {
                    Log.e(TAG, "Error reading from stream", e)
                    break
                }
            }
        }

        fun write(message: String) {
            try {
                outStream.write(message.toByteArray())
            } catch (e: IOException) {
                Log.e(TAG, "Error writing to stream", e)
            }
        }

        fun cancel() {
            try {
                socket.close()
            } catch (e: IOException) {
                Log.e(TAG, "Error closing socket", e)
            }
        }
    }
    private fun processBluetoothMessage(message: String) {
        // 블루투스 시리얼 값이 "bright:8"인 경우 tv_BH1750 텍스트를 8로 변경
        if (message.startsWith("bright:")) {
            val value = message.substringAfter("bright:").trim()
            speed.text = value
        }
    }
    override fun onDestroy() {
        super.onDestroy()
        connectedThread?.cancel()
    }
}