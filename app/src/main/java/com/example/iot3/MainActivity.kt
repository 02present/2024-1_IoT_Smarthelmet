package com.example.iot3

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.*
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.ImageView
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.util.*

class MainActivity : AppCompatActivity() {
    private val TAG = "BluetoothArduino"
    private val REQUEST_ENABLE_BT = 1
    private val REQUEST_FINE_LOCATION = 2
    private val DEVICE_ADDRESS = "D8:B6:73:0B:FC:47"
    private val HM10_UUID_SERVICE = UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb")
    private val HM10_UUID_CHARACTERISTIC = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb")

    private lateinit var bluetoothAdapter: BluetoothAdapter
    private var bluetoothGatt: BluetoothGatt? = null
    private lateinit var handler: Handler
    private lateinit var speed: TextView
    private lateinit var bright: TextView
    private lateinit var mpu6050_x: TextView
    private lateinit var mpu6050_y: TextView
    private lateinit var mpu6050_z: TextView
    private lateinit var connectButton: Button
    private lateinit var img_BH1750: ImageView

    companion object {
        const val MESSAGE_READ = 0
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        speed = findViewById(R.id.tv_MPU9250)
        bright = findViewById(R.id.tv_BH1750)
        mpu6050_x = findViewById(R.id.mpu6050_x)
        mpu6050_y = findViewById(R.id.mpu6050_y)
        mpu6050_z = findViewById(R.id.mpu6050_z)
        connectButton = findViewById(R.id.connectbutton)
        img_BH1750 = findViewById(R.id.img_BH1750)

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth not supported", Toast.LENGTH_SHORT).show()
            finish()
            return
        }
        connectToDevice()

        connectButton.setOnClickListener {
            if (bluetoothGatt != null) {
                disconnectFromDevice()
            } else {
                if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), REQUEST_FINE_LOCATION)
                } else {
                    connectToDevice()
                }
            }
        }

        handler = Handler(Looper.getMainLooper())
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            REQUEST_ENABLE_BT -> {
                if (grantResults.isNotEmpty() && grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                    connectToDevice()
                } else {
                    Toast.makeText(this, "Permissions are required for Bluetooth connection", Toast.LENGTH_SHORT).show()
                }
            }
        }
    }

    private fun connectToDevice() {
        val device: BluetoothDevice = bluetoothAdapter.getRemoteDevice(DEVICE_ADDRESS)
        bluetoothGatt = device.connectGatt(this, false, gattCallback)
    }

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                // 연결 성공 시에는 서비스를 발견합니다.
                gatt.discoverServices()
                connectButton.text = "블루투스 연결 해제"
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                // 연결이 끊어졌을 때는 필요한 작업을 수행합니다.
                runOnUiThread {
                    Toast.makeText(this@MainActivity, "Disconnected from device", Toast.LENGTH_SHORT).show()
                    connectButton.text = "블루투스 연결"
                }
                // 필요한 경우 다른 작업 수행
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                val characteristic = gatt.getService(HM10_UUID_SERVICE)?.getCharacteristic(HM10_UUID_CHARACTERISTIC)
                characteristic?.let {
                    gatt.setCharacteristicNotification(it, true)
                    val descriptor = it.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"))
                    descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                    gatt.writeDescriptor(descriptor)
                }
            }
        }

        override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
            val message = characteristic.value.toString(Charsets.UTF_8)
            processBluetoothMessage(message)
        }
    }

    private fun disconnectFromDevice() {
        bluetoothGatt?.disconnect()
    }

    @SuppressLint("SetTextI18n")
    private fun processBluetoothMessage(message: String) {
        val parts = message.split(" ") // 문자열을 공백을 기준으로 분할하여 부분으로 나눔

        for (part in parts) {
            val keyValue = part.split(":") // 부분을 ":"를 기준으로 분할하여 키와 값으로 나눔
            if (keyValue.size == 2) {
                val key = keyValue[0]
                val value = keyValue[1]

                when (key) {
                    "speed" -> speed.text = value + "km/h"
                    "bright" -> bright.text = value
                    //if (value > "50") {
                    //    img_BH1750.src =
                    //}
                    "ax" -> mpu6050_x.text = "x :$value"
                    "ay" -> mpu6050_y.text = "y :$value"
                    "az" -> mpu6050_z.text = "z :$value"
                    // 다른 키에 대한 처리를 여기에 추가할 수 있음
                }
            }
        }
        Toast.makeText(this@MainActivity, "Received: $parts", Toast.LENGTH_SHORT).show()
        Log.d(TAG, "$message")
    }


    override fun onDestroy() {
        super.onDestroy()
        bluetoothGatt?.close()
        bluetoothGatt = null
    }
}
