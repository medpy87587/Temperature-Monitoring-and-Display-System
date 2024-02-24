package com.example.tempreature;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.example.tempreature.R;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {
    TextView textViewTemperature, temp_hour;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textViewTemperature = findViewById(R.id.textViewTemperature);
        temp_hour = findViewById(R.id.hour);

        DatabaseReference databaseReference = FirebaseDatabase.getInstance().getReference();

        databaseReference.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                // Read instant temperature
                Float instantTemperature = dataSnapshot.child("instant_temperature").getValue(Float.class);
                if (instantTemperature != null) {
                    Log.d("Firebase", "Instant Temperature: " + instantTemperature);
                    textViewTemperature.setText("Your house temperature is: " + instantTemperature);
                } else {
                    Log.d("Firebase", "Instant Temperature data does not exist");
                }

            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {
                // Handle potential errors
                Log.d("Firebase", "Database read cancelled: " + databaseError.getMessage());
            }
        });
        databaseReference.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                // Read instant temperature


                // Read hourly mean temperatures
                StringBuilder hourMeanTemperatureInfo = new StringBuilder();
                for (int i = 0; i < 24; i++) {
                    Float meanTemperature = dataSnapshot.child("/temperature/hour_" + i+ "/mean_temperature").getValue(Float.class);
                    if (meanTemperature != null) {
                        Log.d("Firebase", "Mean Temperature for hour " + i + ": " + meanTemperature);
                        hourMeanTemperatureInfo.append(String.format("Hour %02d: %.2f °C\n", i, meanTemperature));
                    } else {
                        Log.d("Firebase", "Mean Temperature data does not exist for hour " + i);
                    }
                }
                // Update UI with hourly mean temperature information
                temp_hour.setText(hourMeanTemperatureInfo.toString());
            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {
                // Handle potential errors
                Log.d("Firebase", "Database read cancelled: " + databaseError.getMessage());
            }
        });
    }
}
