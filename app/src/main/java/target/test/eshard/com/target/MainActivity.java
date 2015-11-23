package target.test.eshard.com.target;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        final Button triggerBtn = (Button) findViewById(R.id.trigger);
        triggerBtn.setOnClickListener(new TriggerOnClickListener(this));

    }
}
