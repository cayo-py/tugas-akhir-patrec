#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "model_regresi.h"

// >> KONSFIGURASI JARINGAN
const char* ssid         = "[SSID]";
const char* password     = "[Pass]";
const char* mqtt_broker  = "broker.emqx.io";
const int mqtt_port      = 1883;

// >> TOPIC MQTT 
const char* topic_input  = "buku/prediksi/input";
const char* topic_output = "buku/prediksi/harga";

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// >> FUNGSI INFERENSI REGRESI 
float hitungHargaPrediksi(int halaman, int is_Softcover, int is_Bookpaper, int is_HVS, 
                          int is_Erlangga, int is_Gramedia, int is_MarjinKiri, int is_Mizan) {
    float harga = INTERCEPT 
                  + (COEF_jumlah_halaman * halaman) 
                  + (COEF_jenis_cover_Softcover * is_Softcover) 
                  + (COEF_kualitas_cetak_Bookpaper * is_Bookpaper) 
                  + (COEF_kualitas_cetak_HVS * is_HVS) 
                  + (COEF_penerbit_Erlangga * is_Erlangga) 
                  + (COEF_penerbit_Gramedia_Pustaka_Utama * is_Gramedia) 
                  + (COEF_penerbit_Marjin_Kiri * is_MarjinKiri) 
                  + (COEF_penerbit_Mizan * is_Mizan);
    return harga;
}

// >> FUNGSI SETUP WIFI 
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Menghubungkan ke ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi terhubung");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// >> FUNGSI CALLBACK MQTT (TERIMA DATA) 
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Pesan diterima pada topik [");
    Serial.print(topic);
    Serial.println("]");

    // Ubah payload byte jadi string JSON
    String jsonString = "";
    for (unsigned int i = 0; i < length; i++) {
        jsonString += (char)payload[i];
    }
    Serial.println("Payload Teks: " + jsonString);

    // Alokasi memori statis JSON untuk mencegah fragmentasi memori pada ESP32
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        Serial.print("Gagal parsing JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Ekstraksi nilai teks dari JSON payload
    int halaman          = doc["jumlah_halaman"];
    String cover         = doc["jenis_cover"];
    String cetak         = doc["kualitas_cetak"];
    String penerbit      = doc["penerbit"];

    // Pemetaan Teks ke Variabel One-Hot Encoding (Sesuai tahap 1)
    int is_Softcover = (cover == "Softcover") ? 1 : 0;
    int is_Bookpaper = (cetak == "Bookpaper") ? 1 : 0;
    int is_HVS       = (cetak == "HVS") ? 1 : 0;
    
    int is_Erlangga  = (penerbit == "Erlangga") ? 1 : 0;
    int is_Gramedia  = (penerbit == "Gramedia Pustaka Utama") ? 1 : 0;
    int is_MarjinKiri= (penerbit == "Marjin Kiri") ? 1 : 0;
    int is_Mizan     = (penerbit == "Mizan") ? 1 : 0;

    // Proses Inferensi ML lokal di dalam chip ESP32
    float hasil_harga = hitungHargaPrediksi(halaman, is_Softcover, is_Bookpaper, is_HVS, 
                                            is_Erlangga, is_Gramedia, is_MarjinKiri, is_Mizan);

    Serial.print("Hasil Estimasi Harga: Rp ");
    Serial.println(hasil_harga, 2);

    // Kirim Balik Hasil ke Broker MQTT dalam format JSON
    StaticJsonDocument<200> responseDoc;
    responseDoc["status"] = "success";
    responseDoc["harga_prediksi"] = hasil_harga;

    char responseBuffer[200];
    serializeJson(responseDoc, responseBuffer);
    
    mqtt_client.publish(topic_output, responseBuffer);
    Serial.println("Hasil estimasi berhasil dipublikasikan ke MQTT.");
}

// >> REKONEKSI BROKER MQTT 
void reconnect() {
    while (!mqtt_client.connected()) {
        Serial.print("Mencoba koneksi MQTT...");
        // Membuat Client ID acak agar tidak bentrok di broker publik
        String clientId = "ESP32Client-BookRegression-";
        clientId += String(random(0, 0xffff), HEX);
        
        if (mqtt_client.connect(clientId.c_str())) {
            Serial.println("Terhubung ke Broker.");
            // Melakukan subscribe ke topik input
            mqtt_client.subscribe(topic_input);
        } else {
            Serial.print("Gagal, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" Mencoba lagi dalam 5 detik...");
            delay(5000);
        }
    }
}

// >> SETUP UTAMA 
void setup() {
    Serial.begin(115200);
    setup_wifi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(callback);
}

// >> LOOP UTAMA 
void loop() {
    if (!mqtt_client.connected()) {
        reconnect();
    }
    mqtt_client.loop();
}