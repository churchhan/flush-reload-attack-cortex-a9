package target.test.eshard.com.target;

import android.app.Activity;
import android.view.View;
import android.widget.Toast;

/**
 * Created by tiana on 11/18/15.
 */
public class TriggerOnClickListener implements View.OnClickListener {

    static
    {
        System.loadLibrary("target");
    }

    private native void TriggerTargetFunction();


    public TriggerOnClickListener(Activity activity)
    {

    }

    @Override
    public void onClick(View view)
    {
        Toast.makeText(view.getContext(), "Triggering Target Function", Toast.LENGTH_LONG).show();
        TriggerTargetFunction();

    }
}
